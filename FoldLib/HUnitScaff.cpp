#include "HUnitScaff.h"
#include "Numeric.h"
#include "HChainScaff.h"
#include "CliquerAdapter.h"
#include "IntrRect2Rect2.h"
#include "SuperUnitScaff.h"
#include "FoldOptGraph.h"

HUnitScaff::HUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) : UnitScaff(id, ms, ss, mPairs)
{
	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();
}

void HUnitScaff::sortMasters()
{
	masterTimeStamps = getTimeStampsNormalized(masters, masters.front()->mPatch.Normal, timeScale);
	QMultiMap<double, QString> timeMasterMap;
	for (PatchNode* m : masters)
	{
		// base master is the bottom one
		if (masterTimeStamps[m->mID] < ZERO_TOLERANCE_LOW)
			baseMaster = m;

		// time to master
		timeMasterMap.insert(masterTimeStamps[m->mID], m->mID);
	}
	sortedMasters = timeMasterMap.values().toVector();
}

void HUnitScaff::createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs)
{
	// create chain for each slave with its two masters
	for (int i = 0; i < ss.size(); i++)
	{
		QString mid_low = mPairs[i].front();
		QString mid_high = mPairs[i].last();
		if (masterTimeStamps[mid_low] > masterTimeStamps[mid_high]){
			mid_low = mPairs[i].last();
			mid_high = mPairs[i].front();
		}

		// create chain
		PatchNode* master_low = (PatchNode*)getNode(mid_low);
		PatchNode* master_high = (PatchNode*)getNode(mid_high);
		ChainScaff* hc = new HChainScaff(ss[i], master_low, master_high);
		double t0 = 1.0 - masterTimeStamps[mid_high];
		double t1 = 1.0 - masterTimeStamps[mid_low];
		hc->setFoldDuration(t0, t1);
		chains << hc;

		// map from master id to chain idx set
		masterChainsMap[mid_low] << i;
		masterChainsMap[mid_high] << i;

		// map from chain index to top master
		chainTopMasterMap[i] = mid_high;
	}
}

HUnitScaff::~HUnitScaff()
{
	for (HUnitSolution* sln : testedSlns)
		delete sln;
}

void HUnitScaff::initFoldSolution()
{
	// clear
	allFoldOptions.clear();

	// generate all fold options
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (int i = 0; i < chains.size(); i++)
	{
		// regular option
		for (FoldOption* fo : chains[i]->genRegularFoldOptions())
		{
			fo->chainIdx = i;
			fo->regionProj = base_rect.get2DRectangle(fo->region);
			fo->computeCost(chains[i]->importance);
			fo->index = allFoldOptions.size();
			allFoldOptions << fo;
		}

		// delete option
		FoldOption* dfo = chains[i]->genDeleteFoldOption();
		dfo->chainIdx = i;
		dfo->computeCost(chains[i]->importance);
		dfo->index = allFoldOptions.size();
		allFoldOptions << dfo;
	}

	// clear solutions
	for (HUnitSolution* sln : testedSlns)
		delete sln;
	testedSlns.clear();
	currSlnIdx = -1;
}

// The keyframe is the configuration of the block at given time \p t
// This is also called regular keyframe to distinguish from super keyframe
// the keyframe cannot be nullptr
Scaffold* HUnitScaff::getKeyframe(double t, bool useThk)
{
	// keyframe of each chain
	QVector<Scaffold*> chainKeyframes;
	for (int i = 0; i < chains.size(); i++)
	{
		double localT = chains[i]->duration.getLocalTime(t);
		chainKeyframes << chains[i]->getKeyframe(localT, useThk);
	}

	// combine 
	Scaffold* keyframe = new Scaffold(chainKeyframes, baseMaster->mID, masterChainsMap);

	//// thickness of masters
	//if (useThk){
	//	for (Structure::Node* n : keyframe->getNodesWithTag(MASTER_TAG))
	//		((ScaffNode*)n)->setThickness(thickness);
	//}

	// local garbage collection
	for(Scaffold* c : chainKeyframes) delete c;

	// the key frame
	return keyframe;
}

// obstacles are projected onto the unit's baseMaster in the ssKeyframe
void HUnitScaff::computeObstacles(SuperShapeKf* ssKeyframe, HUnitSolution* sln)
{
	// debug
	sln->obstacles.clear(); 

	// obstacles for each top master
	QString baseMid = baseMaster->mID;
	for (PatchNode* topMaster : masters)
	{
		// skip base master
		QString topMid = topMaster->mID;
		if (topMid == baseMid) continue;

		// obstacle points
		QVector<Vector3> obstPnts = computeObstaclePnts(ssKeyframe, baseMid, topMid);

		// projected coordinates onto the base rect
		QVector<Vector2> obstPntsProj;
		for (Vector3 s : obstPnts)
			obstPntsProj << sln->baseRect.getProjCoordinates(s);
		tmObstaclesProj[topMid] = obstPntsProj;

		// debug
		sln->obstacles << obstPnts;
	}
}

void HUnitScaff::computeAvailFoldOptions(SuperShapeKf* ssKeyframe, HUnitSolution* sln)
{
	{// debug
		visDebug.clearAll();
	}

	// update obstacles
	computeObstacles(ssKeyframe, sln);

	{// debug
		visDebug.addPoints(sln->obstacles, Qt::blue);
	}

	// prune fold options
	sln->afoIndices.clear();
	for (int i = 0; i < allFoldOptions.size(); i++)
	{
		// the fold option
		FoldOption* fo = allFoldOptions[i];

		// always accept the delete fold option
		// check acceptance for regular fold option
		bool accepted = true;
		if (fo->scale != 0)
		{
			// the obstacles
			QString top_mid = chainTopMasterMap[fo->chainIdx];
			QVector<Vector2> &obs = tmObstaclesProj[top_mid];

			// prune
			double thr = 0.05;
			bool isColliding = fo->regionProj.containsAny(obs, -thr);
			bool inAABB = sln->aabbCstrProj.containsAll(fo->regionProj.getConners(), thr);
			accepted = !isColliding && inAABB;

			{// debug
				Geom::Rectangle tmRect = sln->baseRect.get3DRectangle(fo->regionProj);
				if (accepted)
					visDebug.addRectangle(tmRect, Qt::green);
				//else
					//visDebug.addRectangle(tmRect, Qt::red);
			}
		}

		// store
		if (accepted) sln->afoIndices << i;
	}
}

bool HUnitScaff::findExistedSolution(HUnitSolution* sln)
{
	currSlnIdx = -1;
	for (int i = 0; i < testedSlns.size(); i++)
	{
		HUnitSolution* tSln = testedSlns[i];
		if (tSln->afoIndices == sln->afoIndices)
		{
			// update the obstacles for debug
			tSln->obstacles = sln->obstacles;

			// set the current
			currSlnIdx = i;

			// return the cost
			return true;
		}
	}

	return false;
}

FoldOptGraph* HUnitScaff::createCollisionGraph(const QVector<int>& afo)
{
	FoldOptGraph* collFog = new FoldOptGraph();

	// chain nodes
	QVector<ChainNode*> chainNodes;
	for (int ci = 0; ci < chains.size(); ci++)
	{
		ChainNode* cn = new ChainNode(ci, chains[ci]->mID);
		collFog->addNode(cn);
		chainNodes << cn;
	}

	// fold option nodes and fold edges
	QVector<FoldOption*> fos;
	for (auto fi: afo)
	{
		FoldOption* fo = (FoldOption*)allFoldOptions[fi]->clone();
		fos << fo;
		collFog->addNode(fo);

		ChainNode* cn = chainNodes[fo->chainIdx];
		collFog->addFoldLink(cn, fo);
	}
	
	// collision edges
	for (int i = 0; i < fos.size(); i++)
	{
		// skip delete fold option
		FoldOption* foi = fos[i];
		if (foi->scale == 0) continue;

		// collision with others
		for (int j = i + 1; j < afo.size(); j++)
		{
			// skip delete fold option
			FoldOption* foj = fos[j];
			if (foj->scale == 0) continue;

			// skip siblings
			if (foi->chainIdx == foj->chainIdx) continue;

			// skip if time interval don't overlap
			if (!foi->duration.overlaps(foj->duration)) continue;

			// collision test using fold region
			if (Geom::IntrRect2Rect2::test(foi->regionProj, foj->regionProj))
				collFog->addCollisionLink(foi, foj);
		}
	}

	return collFog;
}

void HUnitScaff::findOptimalSolution(HUnitSolution* sln)
{
	// initial
	sln->cost = 0;
	sln->chainSln.clear();
	for (int i = 0; i < chains.size(); i++) sln->chainSln << -1;

	// optimal solution on each component
	FoldOptGraph* collFog = createCollisionGraph(sln->afoIndices);
	for (auto component : collFog->getComponents()) {
		for (auto fo: findOptimalSolution(collFog, component))
		{
			// chain solution (the index of fold option)
			sln->chainSln[fo->chainIdx] = fo->index;
			sln->cost += fo->cost;
		}
	}

	// garbage collection
	delete collFog;
}

QVector<FoldOption*> HUnitScaff::findOptimalSolution(FoldOptGraph* collFog, const QVector<Structure::Node*>& component)
{
	QVector<FoldOption*> partialSln;

	// all fold options in the component
	// these fold options are from allFoldOptions
	QVector<FoldOption*> fns;
	int nbCn = 0;
	for (auto n : component) 
	{
		if (n->properties["type"].toString() == "option")
		{
			int idx = ((FoldOption*)n)->index;
			fns << allFoldOptions[idx];
		}
		else nbCn++;
	}

	// ***single chain: choose the one with lowest cost
	if (nbCn == 1)
	{
		FoldOption* best_fo = nullptr;
		double minCost = maxDouble();
		for (auto fo : fns)	{
			if (fo->cost < minCost){
				minCost = fo->cost;	best_fo = fo;
			}
		}
		partialSln << best_fo;
	}else
	// ***multiple chains: MIS
	{
		// the dual adjacent matrix and weights for each fold option
		QVector< QVector<bool> > conn = genDualAdjMatrix(collFog, fns);
		QVector<double> weights = genReversedWeights(fns);

		// find maximum weighted clique
		CliquerAdapter cliquer(conn, weights);
		QVector<QVector<int> > qs = cliquer.getMaxWeightedCliques();

		// there might be multiple solution, we simply use the first one
		for (int idx : qs.front())
			partialSln << fns[idx];
	}

	return partialSln;
}

QVector< QVector<bool> > HUnitScaff::genDualAdjMatrix(FoldOptGraph* collFog, const QVector<FoldOption*>& fns)
{
	QVector<bool> dumpy(fns.size(), true);
	QVector< QVector<bool> > conn(fns.size(), dumpy);
	for (int i = 0; i < fns.size(); i++)
	{
		// the i-th node
		FoldOption* fn = fns[i];

		// diagonal entry
		conn[i][i] = false;

		// other fold options
		for (int j = i + 1; j < fns.size(); j++)
		{
			// the j-th node
			FoldOption* other_fn = fns[j];

			// connect siblings and colliding folding options
			if ((fn->chainIdx == other_fn->chainIdx) ||
				collFog->verifyLinkType(fn->mID, other_fn->mID, "collision")){
				conn[i][j] = false;	conn[j][i] = false;
			}
		}
	}

	return conn;
}

// max weights means lowest cost: weights = maxCost - cost
QVector<double> HUnitScaff::genReversedWeights(const QVector<FoldOption*>& fos)
{
	// the max cost
	double maxCost = 0;
	for (FoldOption* fo : fos)
		if (fo->cost > maxCost) maxCost = fo->cost;
	maxCost += 1;

	// reversed weights
	QVector<double> weights;
	for (FoldOption* fo : fos)
		weights.push_back(maxCost - fo->cost);

	return weights;
}


double HUnitScaff::foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti)
{
	// set time interval
	mFoldDuration = ti;

	// create a new solution
	HUnitSolution* fdSln = new HUnitSolution();
	fdSln->baseRect = getBaseRect(ssKeyframe);
	fdSln->aabbCstrProj = getAabbCstrProj(fdSln->baseRect);

	// available fold options
	computeAvailFoldOptions(ssKeyframe, fdSln);

	// search for existed solutions
	if (findExistedSolution(fdSln))
	{
		// garbage collection
		delete fdSln;
	}
	else
	{
		// find new solution
		findOptimalSolution(fdSln);

		// store the solution
		currSlnIdx = testedSlns.size();
		testedSlns << fdSln;
	}

	// apply the solution
	HUnitSolution* currSln = testedSlns[currSlnIdx];
	for (int i = 0; i < chains.size(); i++)
	{
		int foIdx = currSln->chainSln[i];
		if (foIdx >= 0 && foIdx < allFoldOptions.size())
			chains[i]->applyFoldOption(allFoldOptions[foIdx]);
		else
			std::cout << "**** no solution for chain = " << chains[i]->mID.toStdString() << "!!!\n";
	}

	// return the cost (\in [0, 1])
	return currSln->cost;
}


QVector<Vector3> HUnitScaff::getCurrObstacles()
{
	QVector<Vector3> obs;
	if (currSlnIdx >= 0 && currSlnIdx < testedSlns.size())
		obs = testedSlns[currSlnIdx]->obstacles;

	return obs;
}

QVector<Geom::Rectangle> HUnitScaff::getCurrAFRs()
{
	QVector<Geom::Rectangle> afr;
	if (currSlnIdx >= 0 && currSlnIdx < testedSlns.size())
	{
		for (int foi : testedSlns[currSlnIdx]->afoIndices)
		{
			Geom::Rectangle baseRect = testedSlns[currSlnIdx]->baseRect;
			Geom::Rectangle2 regionProj = allFoldOptions[foi]->regionProj;
			afr << baseRect.get3DRectangle(regionProj);
		}
	}

	return afr;
}

QVector<Geom::Rectangle> HUnitScaff::getCurrSlnFRs()
{
	QVector<Geom::Rectangle> slnFr;
	if (currSlnIdx >= 0 && currSlnIdx < testedSlns.size())
	{
		for (int idx : testedSlns[currSlnIdx]->chainSln)
		{
			Geom::Rectangle baseRect = testedSlns[currSlnIdx]->baseRect;
			Geom::Rectangle2 regionProj = allFoldOptions[idx]->regionProj;
			slnFr << baseRect.get3DRectangle(regionProj);
		}
	}

	return slnFr;
}
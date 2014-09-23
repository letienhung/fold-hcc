#include "HBlockGraph.h"
#include "Numeric.h"
#include "HChainGraph.h"
#include "CliquerAdapter.h"
#include "IntrRect2Rect2.h"
#include "SuperBlockGraph.h"
#include "FoldOptionGraph.h"

HBlockGraph::HBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
	QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb)
	:BlockGraph(id, shape_aabb)
{
	// clone nodes
	foreach(PatchNode* m, ms)	{
		masters << (PatchNode*)m->clone();
		Structure::Graph::addNode(masters.last());
	}
	foreach(FdNode* s, ss)
		Structure::Graph::addNode(s->clone());

	// sort masters
	masterTimeStamps = getTimeStampsNormalized(masters, masters.front()->mPatch.Normal, timeScale);
	QMultiMap<double, QString> timeMasterMap;
	foreach(PatchNode* m, masters)
	{
		// base master is the bottom one
		if (masterTimeStamps[m->mID] < ZERO_TOLERANCE_LOW)
			baseMaster = m;

		// time to master
		timeMasterMap.insert(masterTimeStamps[m->mID], m->mID);
	}
	sortedMasters = timeMasterMap.values().toVector();

	// create chains
	createChains(ss, mPairs);

	// generate all fold options
	genAllFoldOptions();
}

void HBlockGraph::createChains(QVector<FdNode*>& ss, QVector< QVector<QString> >& mPairs)
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
		ChainGraph* hc = new HChainGraph(ss[i], master_low, master_high);
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

	// normalize patch area
	double totalA = 0;
	foreach(ChainGraph* chain, chains)
		totalA += chain->origSlave->mPatch.area();
	foreach(ChainGraph* chain, chains)
		chain->patchArea /= totalA;
}

HBlockGraph::~HBlockGraph()
{
}

// The keyframe is the configuration of the block at given time \p t
// This is also called regular keyframe to distinguish from super keyframe
FdGraph* HBlockGraph::getKeyframe(double t, bool useThk)
{
	FdGraph* keyframe = nullptr;

	// the block is not ready to fold
	if (t <= 0)
	{
		keyframe = (FdGraph*)this->clone();
	}
	// chains have been created and ready to fold
	// IOW, the block has been foldabilized
	else if (foldabilized)
	{
		// keyframe of each chain
		QVector<FdGraph*> chainKeyframes;
		for (int i = 0; i < chains.size(); i++)
		{
			// skip deleted chain
			if (chains[i]->hasTag(DELETED_TAG))
				chainKeyframes << nullptr;
			else{
				ChainGraph* cgraph = chains[i];
				double localT = getLocalTime(t, cgraph->duration);
				chainKeyframes << chains[i]->getKeyframe(localT, useThk);
			}
		}

		// combine 
		keyframe = combineFdGraphs(chainKeyframes, baseMaster->mID, masterChainsMap);

		// thickness of masters
		if (useThk){
			foreach(PatchNode* m, getAllMasters(keyframe))
				m->setThickness(thickness);
		}

		// local garbage collection
		foreach(FdGraph* c, chainKeyframes)
		if (c) delete c;
	}

	// keyframe == nullptr only if this block has not been foldabilized but t > 0
	// should never happen

	// the key frame
	return keyframe;
}

void HBlockGraph::computeObstacles(ShapeSuperKeyframe* ssKeyframe)
{
	// create super block
	SuperBlockGraph *superBlock = new SuperBlockGraph(this, ssKeyframe);

	// request from super block
	obstacles = superBlock->computeObstacles();

	// garbage collection
	delete superBlock;
}

QVector<int> HBlockGraph::getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe)
{
	// update obstacles
	computeObstacles(ssKeyframe);

	// prune fold options
	QVector<int> afo;
	for (int i = 0; i < allFoldOptions.size(); i++)
	{
		// the fold option
		FoldOption* fo = allFoldOptions[i];

		// the obstacles
		QString top_mid = chainTopMasterMap[fo->chainIdx];
		QVector<Vector2> &obs = obstacles[top_mid];

		// prune
		if (!fo->regionProj.containsAny(obs, -0.01))
			afo << i;
	}

	// result
	return afo;
}

FoldOptionGraph* HBlockGraph::createCollisionGraph(const QVector<int>& afo)
{
	FoldOptionGraph* collFog = new FoldOptionGraph();

	// chain nodes
	QVector<ChainNode*> chainNodes;
	for (int ci = 0; ci < chains.size(); ci++)
	{
		ChainNode* cn = new ChainNode(ci, chains[ci]->mID);
		collFog->addNode(cn);
		chainNodes << cn;
	}

	// fold option nodes and fold edges
	for (auto fi: afo)
	{
		FoldOption* fo = allFoldOptions[fi];
		ChainNode* cn = chainNodes[fo->chainIdx];

		collFog->addNode(fo);
		collFog->addFoldLink(cn, fo);
	}
	
	// collision edges
	for (int i = 0; i < afo.size(); i++)
	{
		// skip delete fold option
		FoldOption* foi = allFoldOptions[i];
		if (foi->hasTag(DELETE_FOLD_OPTION)) continue;

		// collision with others
		for (int j = i + 1; j < afo.size(); j++)
		{
			// skip delete fold option
			FoldOption* foj = allFoldOptions[j];
			if (foj->hasTag(DELETE_FOLD_OPTION)) continue;

			// skip siblings
			if (foi->chainIdx == foj->chainIdx) continue;

			// skip if time interval don't overlap
			if (!overlap(foi->duration, foj->duration)) continue;

			// collision test using fold region
			if (Geom::IntrRect2Rect2::test(foi->regionProj, foj->regionProj))
				collFog->addCollisionLink(foi, foj);
		}
	}

	return collFog;
}

double HBlockGraph::findOptimalSolution(const QVector<int>& afo)
{
	// total cost
	double totalCost = -1;

	// solution
	QVector<FoldOption*> solution;
	for (int i = 0; i < chains.size(); i++) solution << nullptr;

	// optimal solution on each component
	FoldOptionGraph* collFog = createCollisionGraph(afo);
	for (auto component : collFog->getComponents()) {
		for (auto fo: findOptimalSolution(collFog, component))
		{
			solution[fo->chainIdx] = fo;
			totalCost += fo->getCost(weight);
		}
	}

	// store the solution
	testedAvailFoldOptions << afo;
	foldSolutions << solution;

	// update the current solution
	currSlnIdx = foldSolutions.size();

	// garbage collection
	delete collFog;

	// returns the cost
	return totalCost;
}

QVector<FoldOption*> HBlockGraph::findOptimalSolution(FoldOptionGraph* collFog, const QVector<Structure::Node*>& component)
{
	QVector<FoldOption*> partialSln;

	// all fold options in the component
	QVector<FoldOption*> fns;
	int nbCn = 0;
	for (auto n : component) 
	{
		if (n->properties["type"].toString() == "option")
			fns << (FoldOption*)n;
		else nbCn++;
	}

	// ***single chain: choose the one with lowest cost
	if (nbCn == 1)
	{
		FoldOption* best_fo = nullptr;
		double minCost = maxDouble();
		for (auto fo : fns)	{
			double cost = fo->getCost(weight);
			if (cost < minCost){
				minCost = cost;	best_fo = fo;
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
		foreach(int idx, qs.front())
			partialSln << fns[idx];
	}

	return partialSln;
}

QVector< QVector<bool> > HBlockGraph::genDualAdjMatrix(FoldOptionGraph* collFog, const QVector<FoldOption*>& fns)
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
QVector<double> HBlockGraph::genReversedWeights(const QVector<FoldOption*>& fns)
{
	QVector<double> weights;

	double maxCost = 0;
	QVector<double> costs;
	foreach(FoldOption* fn, fns)
	{
		double cost = fn->getCost(weight);
		costs << cost;
		if (cost > maxCost) maxCost = cost;
	}
	maxCost += 1;

	foreach(double cost, costs) 
		weights.push_back(maxCost - cost);

	return weights;
}

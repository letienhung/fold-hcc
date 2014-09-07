#include "HBlockGraph.h"
#include "Numeric.h"
#include "ChainGraph.h"
#include "CliquerAdapter.h"
#include "IntrRect2Rect2.h"

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

		// height
		masterHeight[m->mID] = masterTimeStamps[m->mID] * timeScale;

		// time to master
		timeMasterMap.insert(masterTimeStamps[m->mID], m->mID);
	}
	sortedMasters = timeMasterMap.values().toVector();

	// create chains
	createChains(ss, mPairs);

	// initial collision graph
	collFog = new FoldOptionGraph();

	// super block
	superBlock = nullptr;
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
		ChainGraph* hc = new ChainGraph(ss[i], master_low, master_high);
		double t0 = 1.0 - masterTimeStamps[mid_high];
		double t1 = 1.0 - masterTimeStamps[mid_low];
		hc->setFoldDuration(t0, t1);
		chains << hc;

		// map from master id to chain idx set
		masterChainsMap[mid_low] << i;
		masterChainsMap[mid_high] << i;

		// map from master id to under chain ids set
		masterUnderChainsMap[mid_high] << i;
		chainBaseMasterMap[i] = mid_low;
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
	if (collFog) delete collFog;

	if (superBlock) delete superBlock;
}

void HBlockGraph::pruneFoldOptions(QVector<FoldOption*>& options, int cid)
{
	// superBlock must be ready
	if (superBlock == nullptr) return;

	// AFR
	ChainGraph* chain = chains[cid];
	QString top_mid_super = superBlock->chainTopMasterMap[cid];
	Geom::Rectangle2 AFR = superBlock->availFoldingRegion[top_mid_super];
	AFR.Extent *= 1.01; // ugly way to avoid numerical issue

	// filter
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QVector<FoldOption*> options_pruned;
	foreach(FoldOption* fn, options)
	{
		bool reject = false;

		// reject if exceeds AFR
		Geom::Rectangle2 fRegion2 = base_rect.get2DRectangle(fn->region);
		if (!AFR.containsAll(fRegion2.getConners()))
			reject = true;

		// reject if collide with other masters
		// whose time stamp is within the time interval of fn
		if (!reject)
		{
			foreach(QString mid, masterTimeStamps.keys())
			{
				double mstamp = masterTimeStamps[mid];
				if (!within(1 - mstamp, chain->duration)) continue;

				Geom::Rectangle m_rect = ((PatchNode*)getNode(mid))->mPatch;
				if (fAreasIntersect(fn->region, m_rect))
				{
					reject = true;
					break;
				}
			}
		}

		// reject or accept
		if (reject)	delete fn;
		else options_pruned << fn;
	}

	// store
	options = options_pruned;
}

void HBlockGraph::addNodesToCollisionGraph()
{
	// fold entities and options
	QVector<Geom::Rectangle> frs;
	Geom::Rectangle base_rect = baseMaster->mPatch;

	// fold options
	for (int cid = 0; cid < chains.size(); cid++)
	{
		// the chain
		std::cout << "cid = " << cid << ": ";
		ChainGraph* chain = (ChainGraph*)chains[cid];

		// fold entity
		ChainNode* cn = new ChainNode(cid, chain->mID);
		collFog->addNode(cn);

		// fold options
		// nS: # splits; nC: # used chunks; nbChunks: total # of chunks
		// enumerate all start positions and left/right side
		// H-chain has min nS = 1 and are odd numbers, min nC = 1
		QVector<FoldOption*> options;
		for (int nS = 1; nS <= nbSplits; nS += 2)
		for (int nC = 1; nC <= nbChunks; nC++)
			options << chain->genFoldOptions(nS, nC, nbChunks);

		// fold region and duration
		foreach(FoldOption* fn, options)
		{
			fn->region = chain->getFoldRegion(fn);
			fn->duration = chain->duration;
		}

		// prune fold options using AFS
		std::cout << "#options = " << options.size();
		pruneFoldOptions(options, cid);
		std::cout << " ==> " << options.size() << std::endl;

		// "delete" option
		options << chain->generateDeleteFoldOption(nbSplits);

		// add to collision graph and link to chain node
		foreach(FoldOption* fn, options)
		{
			collFog->addNode(fn);
			collFog->addFoldLink(cn, fn);
		}

		// debug
		//foreach (FoldOption* fn, options) frs << fn->region;
		//frs.remove(options.size()-1); // last one is delete option
	}

	// debug
	//properties[FOLD_REGIONS].setValue(frs);
}

void HBlockGraph::addEdgesToCollisionGraph()
{
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
		// skip delete fold option
		if (fns[i]->hasTag(DELETE_FOLD_OPTION)) continue;

		// collision with others
		for (int j = i + 1; j < fns.size(); j++)
		{
			// skip delete fold option
			if (fns[j]->hasTag(DELETE_FOLD_OPTION)) continue;

			// skip siblings
			if (collFog->areSiblings(fns[i]->mID, fns[j]->mID)) continue;

			// skip if time interval don't overlap
			if (!overlap(fns[i]->duration, fns[j]->duration)) continue;

			// collision test using fold region
			if (fAreasIntersect(fns[i]->region, fns[j]->region))
				collFog->addCollisionLink(fns[i], fns[j]);
		}
	}
}

void HBlockGraph::exportCollFOG()
{
	QString filename = path + "/" + mID;
	collFog->saveAsImage(filename);
}

void HBlockGraph::foldabilize(ShapeSuperKeyframe* ssKeyframe)
{
	// available folding region
	computeAvailFoldingRegion(ssKeyframe);

	// collision graph
	std::cout << "\n==build collision graph==\n";
	collFog->clear();

	std::cout << "===add nodes===\n";
	addNodesToCollisionGraph();

	std::cout << "===add edges===\n";
	addEdgesToCollisionGraph();

	// find optimal solution
	std::cout << "\n==maximum idependent set==\n";
	findOptimalSolution();

	// apply fold options
	std::cout << "\n==apply solution==\n";
	applySolution(0);
}

void HBlockGraph::findOptimalSolution()
{
	// clear
	foldSolutions.clear();
	QVector<FoldOption*> solution;
	for (int i = 0; i < chains.size(); i++) solution << NULL;

	// MIS on each component
	QVector<QVector<Structure::Node*> > components = collFog->getComponents();
	foreach(QVector<Structure::Node*> collComponent, components)
	{
		// all folding nodes
		QVector<FoldOption*> fns;
		foreach(Structure::Node* n, collComponent)
		if (n->properties["type"].toString() == "option")
			fns << (FoldOption*)n;

		// empty: impossible to happen
		if (fns.isEmpty())
		{
			std::cout << mID.toStdString() << ": collision graph has empty component.\n";
			return; // halt if happens
		}

		// trivial case
		if (fns.size() == 1)
		{
			FoldOption* fn = fns[0];
			ChainNode* cn = collFog->getChainNode(fn->mID);
			solution[cn->chainIdx] = fn;
			continue;
		}

		// the dual adjacent matrix
		QVector<bool> dumpy(fns.size(), true);
		QVector< QVector<bool> > conn(fns.size(), dumpy);
		for (int i = 0; i < fns.size(); i++){
			// the i-th node
			FoldOption* fn = fns[i];

			// diagonal entry
			conn[i][i] = false;

			// other fold options
			for (int j = i + 1; j < fns.size(); j++){
				// the j-th node
				FoldOption* other_fn = fns[j];

				// connect siblings and colliding folding options
				if (collFog->areSiblings(fn->mID, other_fn->mID) ||
					collFog->verifyLinkType(fn->mID, other_fn->mID, "collision")){
					conn[i][j] = false;	conn[j][i] = false;
				}
			}
		}

		// cost and weight for each fold option
		double maxCost = 0;
		QVector<double> costs;
		foreach(FoldOption* fn, fns){
			double cost = fn->getCost(weight);
			costs << cost;
			if (cost > maxCost) maxCost = cost;
		}
		maxCost += 1;
		QVector<double> weights;
		foreach(double cost, costs) weights.push_back(maxCost - cost);

		// find maximum weighted clique
		CliquerAdapter cliquer(conn, weights);
		QVector<QVector<int> > qs = cliquer.getMaxWeightedCliques();
		if (!qs.isEmpty()) {
			foreach(int idx, qs.front())
			{
				FoldOption* fn = fns[idx];
				ChainNode* cn = collFog->getChainNode(fn->mID);
				solution[cn->chainIdx] = fn;
			}
		}
	}

	// save the single solution
	foldSolutions << solution;
}

void HBlockGraph::applySolution(int idx)
{
	// assert idx
	if (idx < 0 || idx >= foldSolutions.size()) return;

	// clear selection in collision graph
	foreach(Structure::Node* n, collFog->nodes)
		n->removeTag(SELECTED_TAG);

	// update selection index
	selSlnIdx = idx;

	// apply fold option to each chain
	for (int i = 0; i < chains.size(); i++)
	{
		FoldOption* fn = foldSolutions[selSlnIdx][i];

		if (fn)
		{
			chains[i]->applyFoldOption(fn);
			fn->addTag(SELECTED_TAG);
		}
	}

	// has been foldabilized
	foldabilized = true;
}

bool HBlockGraph::fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;

	Geom::Rectangle2 r1 = base_rect.get2DRectangle(rect1);
	Geom::Rectangle2 r2 = base_rect.get2DRectangle(rect2);

	return Geom::IntrRect2Rect2::test(r1, r2);
}

void HBlockGraph::computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe)
{
	// update super block
	if (superBlock != nullptr) delete superBlock;
	superBlock = new SuperBlockGraph(this, ssKeyframe);

	// compute AFR
	superBlock->computeAvailFoldingRegion();

	// store the region
	availFoldingRegion = superBlock->getAvailFoldingRegion();

	// debug AFS
	//properties[AFS].setValue(superBlock->getAllAFS());
}

// The keyframe is the configuration of the block at given time \p t
// This is also called regular keyframe to distinguish from super keyframe
FdGraph* HBlockGraph::getKeyframe(double t, bool useThk)
{
	FdGraph* keyframe = NULL;

	// chains have been created and ready to fold
	// IOW, the block has been foldabilized
	if (foldabilized)
	{
		// keyframe of each chain
		QVector<FdGraph*> chainKeyframes;
		for (int i = 0; i < chains.size(); i++)
		{
			// skip deleted chain
			if (chains[i]->hasTag(DELETED_TAG))
				chainKeyframes << NULL;
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
	else
		// the block is not ready
		// can only answer request on t = 0 and t = 1
	{
		// clone
		keyframe = (FdGraph*)this->clone();

		// collapse all masters to base
		if (t > 0.5)
		{
			Geom::Rectangle base_rect = baseMaster->mPatch;
			foreach(FdNode* n, keyframe->getFdNodes())
			{
				// skip base master
				if (n->mID == baseMaster->mID)	continue;

				// translate all other masters onto the base master
				if (n->hasTag(MASTER_TAG))
				{
					Vector3 c2c = baseMaster->center() - n->center();
					Vector3 up = base_rect.Normal;
					Vector3 offset = dot(c2c, up) * up;
					n->translate(offset);
				}
				else
					// remove slave nodes
				{
					keyframe->removeNode(n->mID);
				}
			}
		}
	}

	// the key frame
	return keyframe;
}

double HBlockGraph::getAvailFoldingVolume()
{
	return superBlock->getAvailFoldingVolume();
}

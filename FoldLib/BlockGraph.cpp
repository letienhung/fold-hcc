#include "BlockGraph.h"
#include "FdUtility.h"
#include "IntrRectRect.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "ChainGraph.h"
#include "CliquerAdapter.h"

BlockGraph::BlockGraph( QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss, 
	QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb )
	: FdGraph(id)
{
	// selected chain
	selChainIdx = -1;

	// clone nodes
	foreach (PatchNode* m, ms)	{
		masters << (PatchNode*)m->clone();
		Structure::Graph::addNode(masters.last());
	}
	foreach (FdNode* s, ss) 
		Structure::Graph::addNode(s->clone());

	// sort masters
	masterTimeStamps = getTimeStampsNormalized(masters, masters.front()->mPatch.Normal, timeScale);
	QMultiMap<double, QString> timeMasterMap;
	foreach (PatchNode* m, masters)
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
	for (int i = 0; i < ss.size(); i++)
	{
		QString mid_low = mPairs[i].front();
		QString mid_high = mPairs[i].last();
		if (masterTimeStamps[mid_low] > masterTimeStamps[mid_high]){
			mid_low = mPairs[i].last();
			mid_high = mPairs[i].front();
		}

		// create chain
		ChainGraph* hc = new ChainGraph(ss[i], (PatchNode*)getNode(mid_low), (PatchNode*)getNode(mid_high));
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
	foreach (ChainGraph* chain, chains)
		totalA += chain->origSlave->mPatch.area();
	foreach (ChainGraph* chain, chains)
		chain->patchArea /= totalA;

	// initial collision graph
	collFog = new FoldOptionGraph();

	// shape AABB
	shapeAABB = shape_aabb;
	shapeAABB.Extent[2] *= 2;;

	// super block
	superBlock = nullptr;

	// parameters for deformation
	nbSplits = 1;
	nbChunks = 2;

	// thickness
	thickness = 2;
	useThickness = false;

	// selected solution
	selSlnIdx = -1;

	// cost weight
	w = 0.05;

	// single block
	isAlone = false;

	// foldabilized tag
	foldabilized = false;
}

BlockGraph::~BlockGraph()
{
	foreach(ChainGraph* c, chains)
		delete c;

	if (collFog) delete collFog;

	if (superBlock) delete superBlock;
}

FdGraph* BlockGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

ChainGraph* BlockGraph::getSelChain()
{
	if (selChainIdx >= 0 && selChainIdx < chains.size())
		return chains[selChainIdx];
	else
		return NULL;
}

void BlockGraph::selectChain( QString id )
{
	selChainIdx = -1;
	for (int i = 0; i < chains.size(); i++)
	{
		if (chains[i]->mID == id)
		{
			selChainIdx = i;
			break;
		}
	}
}

QStringList BlockGraph::getChainLabels()
{
	QStringList labels;
	foreach(FdGraph* c, chains)
		labels.push_back(c->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

void BlockGraph::exportCollFOG()
{
	QString filename = path + "/" + mID;
	collFog->saveAsImage(filename);
}

// The keyframe is the configuration of the block at given time \p t
// This is also called regular keyframe to distinguish from super keyframe
FdGraph* BlockGraph::getKeyframe( double t, bool useThk )
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
			foreach (PatchNode* m, getAllMasters(keyframe))
				m->setThickness(thickness);
		}

		// local garbage collection
		foreach (FdGraph* c, chainKeyframes) 
			if(c) delete c;
	}else
	// the block is not ready
	// can only answer request on t = 0 and t = 1
	{
		// clone
		keyframe = (FdGraph*)this->clone();

		// collapse all masters to base
		if (t > 0.5)
		{
			Geom::Rectangle base_rect = baseMaster->mPatch;
			foreach (FdNode* n, keyframe->getFdNodes())
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
				}else
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

// The super keyframe is the keyframe + superPatch
// which is an additional patch representing the folded block
FdGraph* BlockGraph::getSuperKeyframe( double t )
{
	// regular key frame w\o thickness
	FdGraph* keyframe = getKeyframe(t, false);

	// do nothing if the block is NOT fully folded
	if (1 - t > ZERO_TOLERANCE_LOW) return keyframe;

	// create super patch
	PatchNode* superPatch = (PatchNode*)baseMaster->clone();
	superPatch->mID = mID + "_super";
	superPatch->addTag(SUPER_PATCH_TAG); 

	// collect projections of all nodes on baseMaster
	Geom::Rectangle base_rect = superPatch->mPatch;
	QVector<Vector2> projPnts2 = base_rect.get2DConners();
	if (foldabilized)
	{
		// all nodes since they are folded flatly already
		foreach (FdNode* n, keyframe->getFdNodes())
		{
			if (n->mType == FdNode::PATCH)
			{
				Geom::Rectangle part_rect = ((PatchNode*)n)->mPatch;
				projPnts2 << base_rect.get2DRectangle(part_rect).getConners();
			}
			else
			{
				Geom::Segment part_rod = ((RodNode*)n)->mRod;
				projPnts2 << base_rect.getProjCoordinates(part_rod.P0);
				projPnts2 << base_rect.getProjCoordinates(part_rod.P1);
			}
		}
	}
	else if (superBlock != nullptr)
	{
		// guess the folded state using available folding regions
		// this is the special case for predicting folding 
		// ***superBlock must be ready at this point
		foreach (QString mid, superBlock->availFoldingRegion.keys())
			projPnts2 << superBlock->availFoldingRegion[mid].getConners();
	}

	// resize super patch
	Geom::Rectangle2 aabb2 = computeAABB2D(projPnts2);
	superPatch->resize(aabb2);

	// merged parts
	foreach (FdNode* n, keyframe->getFdNodes())
	{
		n->addTag(MERGED_PART_TAG); 
		if (n->mType == FdNode::PATCH)
			superPatch->appendToSetProperty<QString>(MERGED_MASTERS, n->mID);
	}

	// add super patch to keyframe
	keyframe->Structure::Graph::addNode(superPatch);

	return keyframe;
}

bool BlockGraph::fAreasIntersect( Geom::Rectangle& rect1, Geom::Rectangle& rect2 )
{
	Geom::Rectangle base_rect = baseMaster->mPatch;

	Geom::Rectangle2 r1 = base_rect.get2DRectangle(rect1);
	Geom::Rectangle2 r2 = base_rect.get2DRectangle(rect2);

	return Geom::IntrRect2Rect2::test(r1, r2);
}

void BlockGraph::computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe)
{
	// update super block
	if (superBlock != nullptr) delete superBlock;
	superBlock = new SuperBlockGraph(this, ssKeyframe);

	// compute AFR
	superBlock->computeAvailFoldingRegion();

	// debug AFS
	properties[AFS].setValue(superBlock->getAllAFS());
}

void BlockGraph::foldabilize(ShapeSuperKeyframe* ssKeyframe)
{
	// available folding region
	computeAvailFoldingRegion(ssKeyframe);

	// special case of T-block with single T-Chain
	if (isTBlock())
	{
		foldabilizeTBlock();
		return;
	}

	// collision graph
	std::cout << "\n==build collision graph==\n";
	collFog->clear();
	std::cout << "===add nodes===\n";
	addNodesToCollisionGraph();
	std::cout << "===add edges===\n";
	addEdgesToCollisionGraph();

	//exportCollFOG();

	// find optimal solution
	std::cout << "\n==maximum idependent set==\n";
	findOptimalSolution();

	// apply fold options
	std::cout << "\n==apply solution==\n";
	applySolution(0);
}

void BlockGraph::applySolution( int sid )
{
	// clear selection in collision graph
	foreach (Structure::Node* n, collFog->nodes)
		n->removeTag(SELECTED_TAG);

	// assert index
	if (sid < 0 || sid >= foldSolutions.size())
		return;

	// apply fold option to each chain
	selSlnIdx = sid;
	for (int i = 0; i < chains.size(); i++)
	{
		FoldOption* fn = foldSolutions[sid][i];
		
		if(fn)	
		{
			chains[i]->applyFoldOption(fn);
			fn->addTag(SELECTED_TAG);
		}
	}

	// has been foldabilized
	foldabilized = true;
}

void BlockGraph::filterFoldOptions( QVector<FoldOption*>& options, int cid )
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
	QVector<FoldOption*> options_filtered;
	foreach (FoldOption* fn, options)
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
			foreach (QString mid, masterTimeStamps.keys())
			{
				double mstamp = masterTimeStamps[mid];
				if (!within(1-mstamp, chain->duration)) continue;

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
		else options_filtered << fn;
	}

	// store
	options = options_filtered;
}

void BlockGraph::addNodesToCollisionGraph()
{
	// fold entities and options
	QVector<Geom::Rectangle> frs;
	Geom::Rectangle base_rect = baseMaster->mPatch;

	// fold options
	for(int cid = 0; cid < chains.size(); cid++)
	{
		// the chain
		std::cout << "cid = " << cid << ": ";
		ChainGraph* chain = (ChainGraph*)chains[cid];

		// fold entity
		ChainNode* cn = new ChainNode(cid, chain->mID);
		collFog->addNode(cn);

		// fold options
		QVector<FoldOption*> options;
		for (int nS = 1; nS <= nbSplits; nS += 2)
			for (int nUsedChunks = nbChunks; nUsedChunks >= 1; nUsedChunks-- )
					options << chain->generateFoldOptions(nS, nUsedChunks, nbChunks);

		// fold region and duration
		foreach (FoldOption* fn, options) 
		{
			fn->region = chain->getFoldRegion(fn);
			fn->duration = chain->duration;
		}

		// filter fold options using AFS
		std::cout << "#options = " << options.size();
		filterFoldOptions(options, cid);
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

void BlockGraph::addEdgesToCollisionGraph()
{
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
		// skip delete fold option
		if (fns[i]->hasTag(DELETE_FOLD_OPTION)) continue;

		// collision with others
		for (int j = i+1; j < fns.size(); j++)
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

void BlockGraph::findOptimalSolution()
{
	// clear
	foldSolutions.clear();
	QVector<FoldOption*> solution;
	for (int i = 0; i < chains.size(); i++) solution << NULL;

	// MIS on each component
	QVector<QVector<Structure::Node*> > components = collFog->getComponents();
	foreach (QVector<Structure::Node*> collComponent, components)
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
			for (int j = i+1; j < fns.size(); j++){
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
		foreach (FoldOption* fn, fns){
			double cost = computeCost(fn);
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

double BlockGraph::getTimeLength()
{
	return nbMasters(this) - 1;
}

double BlockGraph::getAvailFoldingVolume()
{
	return superBlock->getAvailFoldingVolume();
}

void BlockGraph::setThickness( double thk )
{
	thickness = thk;
	foreach (ChainGraph* chain, chains)	
	{
		chain->halfThk = thickness / 2;
		chain->baseOffset = thickness / 2;
	}
}

double BlockGraph::computeCost( FoldOption* fn )
{
	// cost of splitting
	double cost1 = fn->nSplits;

	// cost of shrinking
	double s = 1 - fn->scale;
	double cost2 = fn->patchArea * s * s;

	// blended cost
	double cost = w * cost1 + (1-w) * cost2;
	return cost;
}

bool BlockGraph::isTBlock()
{
	return (chains.size() == 1 && chains.front()->isTChain());
}

void BlockGraph::foldabilizeTBlock()
{
	// the chain
	ChainGraph* chain = chains.front();

	// generate T-options
	QVector<FoldOption*> options;
	for (int nS = 1; nS <= nbSplits; nS ++)
		for (int nUsedChunks = nbChunks; nUsedChunks >= 1; nUsedChunks-- )
			options << chain->generateFoldOptions(nS, nUsedChunks, nbChunks);

	// filter by AFS
	QVector<FoldOption*> valid_options;
	QString top_mid_super = superBlock->chainTopMasterMap[0];
	Geom::Rectangle2 AFR = superBlock->availFoldingRegion[top_mid_super];
	AFR.Extent *= 1.01; // ugly way to avoid numerical issue
	Geom::Rectangle base_rect = baseMaster->mPatch;
	foreach (FoldOption* fn, options)
	{
		Geom::Rectangle2 fRegion2 = base_rect.get2DRectangle(fn->region);
		if (AFR.containsAll(fRegion2.getConners()))
			valid_options << fn;
	}

	// choose the one with the lowest cost
	FoldOption* best_fn;
	double minCost = maxDouble();
	foreach (FoldOption* fn, valid_options){
		double cost = computeCost(fn);
		if (cost < minCost) 
		{
			best_fn = fn;
			minCost = cost;
		}
	}
	best_fn->addTag(SELECTED_TAG);

	// apply fold option
	chain->applyFoldOption(best_fn);
	foldabilized = true;
}

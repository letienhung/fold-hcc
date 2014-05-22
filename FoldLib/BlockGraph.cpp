#include "BlockGraph.h"
#include "FdUtility.h"
#include "IntrRectRect.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "HChain.h"
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
	foreach (PatchNode* m, masters)
	{
		// base master is the bottom one
		if (masterTimeStamps[m->mID] < ZERO_TOLERANCE_LOW)
			baseMaster = m;

		// height
		masterHeight[m->mID] = masterTimeStamps[m->mID] * timeScale;
	}

	// create chains
	for (int i = 0; i < ss.size(); i++)
	{
		QString mid_low = mPairs[i].front();
		QString mid_high = mPairs[i].last();
		if (masterTimeStamps[mid_low] > masterTimeStamps[mid_high])
		{
			mid_low = mPairs[i].last();
			mid_high = mPairs[i].front();
		}

		// create chain
		HChain* hc = new HChain(ss[i], (PatchNode*)getNode(mid_low), (PatchNode*)getNode(mid_high));
		hc->setFoldDuration(masterTimeStamps[mid_low], masterTimeStamps[mid_high]);
		chains << hc;

		// map from master id to chain idx set
		masterChainsMap[mid_low] << i;
		masterChainsMap[mid_high] << i;

		// map from master id to under chain ids set
		masterUnderChainsMap[mid_high] << i;
	}

	// initial collision graph
	collFog = new FoldOptionGraph();

	// shape AABB
	shapeAABB = shape_aabb;

	// super block
	superBlock = NULL;

	//////////////////////////////////////////////////////////////////////////
	//QVector<Geom::Segment> normals;
	//normals << Geom::Segment(baseMaster->mPatch.Center, baseMaster->mPatch.Center + 10 * baseMaster->mPatch.Normal);
	//addDebugSegments(normals);
}

BlockGraph::~BlockGraph()
{
	foreach(ChainGraph* c, chains)
		delete c;
}

FdGraph* BlockGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

HChain* BlockGraph::getSelChain()
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

void BlockGraph::computeMinFoldingRegion()
{
	Geom::Rectangle base_rect = baseMasterSuper->mPatch;
	foreach (PatchNode* top_master, mastersSuper)
	{
		// skip base master
		if (top_master == baseMasterSuper) continue;

		// min folding region
		Geom::Rectangle2 min_region = base_rect.get2DRectangle(top_master->mPatch);
		minFoldingRegion[top_master->mID] = min_region;

		// debug
		//foreach (int cid, masterUnderChainsMap[top_master->mID])
			//chains[cid]->addDebugSegments(base_rect.get3DRectangle(min_region).getEdgeSegments());
	}
}

void BlockGraph::computeMaxFoldingRegion()
{
	Geom::Rectangle base_rect = baseMasterSuper->mPatch;
	int aid = shapeAABB.getAxisId(base_rect.Normal);
	Geom::Rectangle cropper3 = shapeAABB.getPatch(aid, 0);
	Geom::Rectangle2 cropper2 = base_rect.get2DRectangle(cropper3);
	foreach (PatchNode* top_master, mastersSuper)
	{
		// skip base master
		if (top_master == baseMasterSuper) continue;

		// 2D points from fold region of slaves and top master
		QVector<Vector2> pnts_proj;
		QVector<Vector3> pnts;
		foreach (int cid, masterUnderChainsMapSuper[top_master->mID]) {
			pnts << chains[cid]->getMaxFoldRegion(true).getConners();
			pnts << chains[cid]->getMaxFoldRegion(false).getConners();
		}
		foreach(Vector3 p, pnts) pnts_proj << base_rect.getProjCoordinates(p);
		pnts_proj << minFoldingRegion[top_master->mID].getConners();

		// max region
		Geom::Rectangle2 max_region = computeAABB2D(pnts_proj);
		max_region.cropByAxisAlignedRectangle(cropper2);
		maxFoldingRegion[top_master->mID] = max_region;

		// debug
		//foreach (int cid, masterUnderChainsMap[top_master->mID])
		//{
		//	chains[cid]->addDebugPoints(pnts);
		//	chains[cid]->addDebugSegments(base_rect.get3DRectangle(max_region).getEdgeSegments());

		//	QVector<Vector3> pnts_proj3;
		//	foreach (Vector2 p2, pnts_proj) pnts_proj3 << base_rect.getPosition(p2);
		//	chains[cid]->addDebugPoints(pnts_proj3);
		//}
	}
}

QVector<QString> BlockGraph::getInbetweenOutsideParts( FdGraph* superKeyframe, QString mid1, QString mid2 )
{
	// time line
	Vector3 sqzV = baseMasterSuper->mPatch.Normal;
	Geom::Line timeLine(Vector3(0, 0, 0), sqzV);

	// position on time line
	FdNode* master1 = (FdNode*)getNode(mid1);
	FdNode* master2 = (FdNode*)getNode(mid2);
	double t0 = timeLine.getProjTime(master1->center());
	double t1 = timeLine.getProjTime(master2->center());
	if (t0 > t1) std::swap(t0, t1);
	TimeInterval m1m2 = TIME_INTERVAL(t0, t1);

	// find parts in between m1 and m2
	QVector<QString> inbetweens;
	foreach (FdNode* n, superKeyframe->getFdNodes())
	{
		// skip parts that has been folded
		if (n->hasTag(FOLDED_TAG)) continue;

		// skip parts in this block
		if (containsNode(n->mID)) continue;

		// master
		if (n->hasTag(MASTER_TAG))
		{
			double t = timeLine.getProjTime(n->center());

			if (within(t, m1m2)) 	inbetweens << n->mID;
		}else
		// slave		
		{
			int aid = n->mBox.getAxisId(sqzV);
			Geom::Segment sklt = n->mBox.getSkeleton(aid);
			double t0 = timeLine.getProjTime(sklt.P0);
			double t1 = timeLine.getProjTime(sklt.P1);
			if (t0 > t1) std::swap(t0, t1);
			TimeInterval ti = TIME_INTERVAL(t0+ZERO_TOLERANCE_LOW, t1-ZERO_TOLERANCE_LOW);

			if (overlap(ti, m1m2))	inbetweens << n->mID;
		}
	}

	return inbetweens;
}

QVector<QString> BlockGraph::getUnrelatedMasters( FdGraph* superKeyframe, QString mid1, QString mid2)
{
	QVector<QString> urMasters;

	// retrieve master order constraints
	StringSetMap moc_greater = superKeyframe->properties[MOC_GREATER].value<StringSetMap>();
	StringSetMap moc_less = superKeyframe->properties[MOC_LESS].value<StringSetMap>();

	// related parts with mid1 and mid2
	QSet<QString> moc1 = moc_greater[mid1] + moc_less[mid1];
	QSet<QString> moc2 = moc_greater[mid2] + moc_less[mid2];	

	// parts unrelated with both
	foreach (Structure::Node* n, superKeyframe->nodes)
	{
		// skip slaves
		if (!n->hasTag(MASTER_TAG)) continue;

		// skip folded masters
		if (n->hasTag(FOLDED_TAG)) continue;

		// accept if not related to any
		if (!moc1.contains(n->mID) && !moc2.contains(n->mID))
			urMasters << n->mID;
	}

	return urMasters;
}

void BlockGraph::computeAvailFoldingRegion( FdGraph* superKeyframe )
{
	// align scaffold with this block
	Vector3 pos_block = baseMaster->center();
	Vector3 pos_keyframe = ((FdNode*)superKeyframe->getNode(baseMaster->mID))->center();
	Vector3 offset = pos_block - pos_keyframe;
	superKeyframe->translate(offset, false);

	// super block and min/max folding regions
	computeSuperBlock(superKeyframe);
	computeMinFoldingRegion();
	computeMaxFoldingRegion();

	// extent 
	QString base_mid = baseMasterSuper->mID;
	Geom::Rectangle base_rect = baseMasterSuper->mPatch;
	foreach (PatchNode* top_master, mastersSuper)
	{
		// skip base master
		if (top_master == baseMasterSuper) continue;
		QString top_mid = top_master->mID;

		// samples from constraint parts: in-between and unordered
		QVector<QString> constraintParts;
		constraintParts << getInbetweenOutsideParts(superKeyframe, base_mid, top_mid);
		constraintParts << getUnrelatedMasters(superKeyframe, base_mid, top_mid);
		QVector<Vector3> samples;
		int nbs = 10;
		foreach(QString nid, constraintParts)
		{
			FdNode* n = (FdNode*)superKeyframe->getNode(nid);
			if (n->mType == FdNode::PATCH)
				samples << ((PatchNode*)n)->mPatch.getEdgeSamples(nbs);
			else
				samples << ((RodNode*)n)->mRod.getUniformSamples(nbs);
		}

		// projection on base_rect
		QVector<Vector2> samples_proj;
		foreach (Vector3 s, samples)
			samples_proj << base_rect.getProjCoordinates(s);

		// max folding region
		samples_proj << maxFoldingRegion[top_mid].getEdgeSamples(nbs);

		// include minFR while exclude all constraint points
		Geom::Rectangle2 avail_region = minFoldingRegion[top_mid];
		extendRectangle2D(avail_region, samples_proj);

		// avail folding region
		double epsilon = 10 * ZERO_TOLERANCE_LOW;
		avail_region.Extent += Vector2(epsilon, epsilon);
		availFoldingRegion[top_mid] = avail_region;

		// debug
		//continue;
		foreach (int cid, masterUnderChainsMap[top_master->mID])
		{
			chains[cid]->properties.remove("debugPoints");
			chains[cid]->properties.remove("debugSegments");

			QVector<Vector3> samples_proj3;
			foreach (Vector2 p2, samples_proj) samples_proj3 << base_rect.getPosition(p2);
			chains[cid]->addDebugPoints(samples_proj3);

			chains[cid]->addDebugSegments(base_rect.get3DRectangle(avail_region).getEdgeSegments());
			//chains[cid]->addDebugSegments(base_rect.get3DRectangle(minFoldingRegion[top_mid]).getEdgeSegments());
		}
	}

	// restore the position of scaffold
	superKeyframe->translate(-offset, false);
}

void BlockGraph::exportCollFOG()
{
	QString filename = path + "/" + mID;
	collFog->saveAsImage(filename);

	for(int i = 0; i < debugFogs.size(); i++)
	{
		debugFogs[i]->saveAsImage(filename + "_debug" + QString::number(i));
	}
}

FdGraph* BlockGraph::getKeyframe( double t )
{
	FdGraph* keyframe = NULL;

	// chains have been created and ready to fold
	if (hasTag(READY_TO_FOLD_TAG))
	{
		// scaffolds from folded chains
		QVector<FdGraph*> foldedChains;
		for (int i = 0; i < chains.size(); i++)
		{
			double localT = getLocalTime(t, chains[i]->mFoldDuration);
			foldedChains << chains[i]->getKeyframeScaffold(localT);
		}

		// combine 
		keyframe = combineDecomposition(foldedChains, baseMaster->mID, masterChainsMap);

		// delete folded chains
		foreach (FdGraph* c, foldedChains) delete c;
	}else
	// the block is not ready
	// can only answer request on t = 0 and t = 1
	{
		// clone
		keyframe = (FdGraph*)this->clone();

		// collapse to base
		Geom::Rectangle base_rect = baseMaster->mPatch;
		if (t > 0.5){
			foreach (FdNode* n, keyframe->getFdNodes()){
				if (n->hasTag(MASTER_TAG)) {
					// leave base master untouched
					if (n->mID == baseMaster->mID)	continue;

					// move other masters onto base master
					Vector3 c2c = baseMaster->center() - n->center();
					Vector3 up = base_rect.Normal;
					Vector3 offset = dot(c2c, up) * up;
					n->translate(offset);
				}else {
					// remove slave nodes
					keyframe->removeNode(n->mID);
				}
			}
		}
	}

	return keyframe;
}


FdGraph* BlockGraph::getSuperKeyframe( double t )
{
	FdGraph* keyframe = getKeyframe(t);

	// do nothing if the block is NOT fully folded
	if (1 - t > ZERO_TOLERANCE_LOW) return keyframe;

	// create super patch
	PatchNode* superPatch = (PatchNode*)baseMaster->clone();
	superPatch->mID = mID + "_super";
	superPatch->addTag(SUPER_PATCH_TAG); 

	// resize super patch
	Geom::Rectangle base_rect = superPatch->mPatch;
	QVector<Vector2> pnts2 = base_rect.get2DConners();
	if (hasTag(READY_TO_FOLD_TAG))
	{// use folded parts
		foreach (Structure::Node* n, keyframe->nodes)
		{
			Geom::Rectangle part_rect = ((PatchNode*)n)->mPatch;
			pnts2 << base_rect.get2DRectangle(part_rect).getConners();
		}
	}
	else
	{// predict using available folding regions
		foreach (QString mid, availFoldingRegion.keys())
			pnts2 << availFoldingRegion[mid].getConners();
	}
	Geom::Rectangle2 aabb2 = computeAABB2D(pnts2);
	superPatch->resize(aabb2);


	// maps between regular and super nodes
	foreach (Structure::Node* n, keyframe->nodes)
	{
		n->addTag(FOLDED_TAG); 
		superPatch->appendToSetProperty<QString>(MERGED_MASTERS, n->mID);
	}

	// add to key frame
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

void BlockGraph::foldabilize()
{
	// collision graph
	//buildCollisionGraphAdaptive();
	buildCollisionGraph();

	// find optimal solution
	findOptimalSolution();

	// apply fold options
	applySolution(0);
}

void BlockGraph::applySolution( int sid )
{
	if (sid < 0 || sid >= foldSolutions.size())
		return;

	for (int i = 0; i < chains.size(); i++)
	{
		FoldOption* fn = foldSolutions[sid][i];

		if (fn == NULL)
			continue;

		chains[i]->applyFoldOption(fn);
	}

	addTag(READY_TO_FOLD_TAG);
}

int BlockGraph::encodeModification( int nX, int nY )
{
	return nX * 100 + nY;
}

void BlockGraph::decodeModification( int mdf, int& nX, int& nY )
{
	nX = mdf / 100;
	nY = mdf % 100;
}

void BlockGraph::genNewModifications( QSet<int>& modifications, int max_nX, int nChunks )
{
	// generate next level of modification
	// folding region from lower level must be fully included by some from higher level
	// folding regions at the same level might overlap but don't include each other
	if (modifications.isEmpty())
	{

		modifications << encodeModification(1, nChunks);
	}
	else
	{
		QSet<int > newModifications;
		foreach (int mdf, modifications)
		{
			int nX, nY;
			decodeModification(mdf, nX, nY);
			// two options
			if (nX < max_nX && nY > 0)
			{
				int new_nX = nX + 1;
				int new_nY = nY - 1;

				newModifications << encodeModification(new_nX, nY)
					<< encodeModification(nX, new_nY);
			}
			// one option: more splits
			else if (nX < max_nX)
				newModifications << encodeModification(nX+1, nY);
			// one option: shrink more
			else if (nY > 0)
				newModifications << encodeModification(nX, nY-1);
		}

		modifications = newModifications;
	}

	// debug
	std::cout << "modification set:";
	foreach(int mdf, modifications)
		std::cout << mdf << "  ";
	std::cout << std::endl;
}

void BlockGraph::filterFoldOptions( QVector<FoldOption*>& options, int cid )
{
	if (cid < 0 || cid >= chains.size()) return;

	HChain* chain = (HChain*)chains[cid];
	QString top_mid = chain->getTopMaster()->mID;
	Geom::Rectangle2 AFR = availFoldingRegion[top_mid]; 
	Geom::Rectangle base_rect = baseMaster->mPatch;

	// filter
	QVector<FoldOption*> options_filtered;
	foreach (FoldOption* fn, options)
	{
		bool reject = false;

		// reject if exceeds AFR
		Geom::Rectangle2 fRegion2 = base_rect.get2DRectangle(fn->region);
		if (!AFR.containsAll(fRegion2.getConners()))
			reject = true;

		//// reject if collide with other masters
		//// whose time stamp is within the time interval of fn
		//if (!reject)
		//{
		//	foreach (QString mid, masterTimeStamps.keys())
		//	{
		//		double mstamp = masterTimeStamps[mid];
		//		if (!within(mstamp, chain->mFoldDuration)) continue;

		//		Geom::Rectangle m_rect = ((PatchNode*)getNode(mid))->mPatch;
		//		if (fAreasIntersect(fn->region, m_rect))
		//		{
		//			reject = true;
		//			break;
		//		}
		//	}
		//}

		// reject or accept
		if (reject)	delete fn;
		else options_filtered << fn;
	}

	// store
	options = options_filtered;
}

void BlockGraph::buildCollisionGraph()
{
	// clear
	collFog->clear();

	std::cout << "build collision graph...\n";

	// fold entities and options
	for(int cid = 0; cid < chains.size(); cid++)
	{
		std::cout << "cid = " << cid << ": ";

		HChain* chain = (HChain*)chains[cid];

		// fold entity
		ChainNode* cn = new ChainNode(cid, chain->mID);
		collFog->addNode(cn);

		properties.remove("debugSegments");// debug

		// fold options and links
		QVector<FoldOption*> options;
		int max_nX = 1;
		int nChunks = 4;
		for (int nX = 1; nX <= max_nX; nX++)
			for (int nUsedChunks = nChunks; nUsedChunks >= 1; nUsedChunks-- )
				options << chain->generateFoldOptions(2*nX-1, nUsedChunks, nChunks);
		std::cout << "#options = " << options.size();
		filterFoldOptions(options, cid);
		std::cout << " ==> " << options.size() << std::endl;
		foreach(FoldOption* fn, options)
		{
			collFog->addNode(fn);
			collFog->addFoldLink(cn, fn);

			// debug
			//chain->addDebugSegments(fn->region.getEdgeSegments());
		}
	}

	// collision links
	updateCollisionLinks();
}

void BlockGraph::buildCollisionGraphAdaptive()
{
	// clear
	collFog->clear();

	// fold entities
	for(int i = 0; i < chains.size(); i++)
		collFog->addNode(new ChainNode(i, chains[i]->mID));

	// chains with no free fold option
	QVector<int> collChainIdx;
	for(int i = 0; i < chains.size(); i++) collChainIdx << i;

	int max_nX = 2;
	int nChunks = 4;
	QSet<int> modifications;
	genNewModifications(modifications, max_nX, nChunks);
	while (!modifications.isEmpty())
	{
		// create new fold options for each colliding chain
		foreach (int cid, collChainIdx){
			HChain* chain = (HChain*)chains[cid];
			ChainNode* cn = (ChainNode*)collFog->getNode(chain->mID);

			// create new fold options using modifications
			QVector<FoldOption*> options;
			foreach (int mdf, modifications){
				int nX, nY;
				decodeModification(mdf, nX, nY);
				int nSplits = 2 * nX - 1;
				int nUsedChunks = nY;
				options << chain->generateFoldOptions(nSplits, nUsedChunks, nChunks);
			}
			filterFoldOptions(options, cid);
			foreach (FoldOption* fn, options){
				fn->addTag(NEW_TAG);
				collFog->addNode(fn);
				collFog->addFoldLink(cn, fn);

				// debug
				//addDebugSegments(fn->region.getEdgeSegments());
			}
		}

		// update collisions for new added fold options
		updateCollisionLinks();

		// debug
		debugFogs << (FoldOptionGraph*)collFog->clone();

		// update colliding chain list
		QVector<int> collChainIdx_copy = collChainIdx;
		collChainIdx.clear();
		foreach (int cid, collChainIdx_copy)
			 if (!collFog->hasFreeFoldOptions(chains[cid]->mID))
				 collChainIdx << cid;

		// all chains have free fold option
		if (collChainIdx.isEmpty()) break;

		// otherwise go next generation
		genNewModifications(modifications, max_nX, nChunks);
	}

	// debug
	std::cout << "# fold options / collision graph size = " 
		<< collFog->getAllFoldOptions().size() << std::endl;
}

void BlockGraph::updateCollisionLinks()
{
	std::cout << "update collision links...\n";
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
		// skip if not new
		//if (!fns[i]->hasTag(NEW_TAG)) continue;

		// collision with others
		for (int j = i+1; j < fns.size(); j++)
		{
			// skip siblings
			if (collFog->areSiblings(fns[i]->mID, fns[j]->mID)) continue; 

			// skip if time interval don't overlap
			if (!overlap(fns[i]->duration, fns[j]->duration)) continue;

			// collision test using fold region
			if (fAreasIntersect(fns[i]->region, fns[j]->region))
				collFog->addCollisionLink(fns[i], fns[j]);
		}

		// remove new tag
		//fns[i]->removeTag(NEW_TAG);
	}
	std::cout << std::endl;
}

void BlockGraph::findOptimalSolution()
{
	// get all folding nodes
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();

	if (fns.isEmpty())
	{
		std::cout << mID.toStdString() << ": collision graph is empty.\n";
		return;
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

	// find minimum cost maximum cliques
	QVector<double> weights;
	foreach(FoldOption* fn, fns) weights.push_back(fn->getCost());
	CliquerAdapter cliquer(conn, weights);
	QVector<QVector<int> > qs = cliquer.getMinWeightMaxCliques();

	// fold solutions
	foldSolutions.clear();
	for (int sid = 0; sid < qs.size(); sid++)
	{
		QVector<FoldOption*> solution;
		for (int i = 0; i < chains.size(); i++) solution << NULL;

		foreach(int idx, qs[sid])
		{
			FoldOption* fn = fns[idx];
			//fn->addTag(SELECTED_TAG);
			ChainNode* cn = collFog->getChainNode(fn->mID);
			solution[cn->chainIdx] = fn;

			//addDebugSegments(fn->region.getEdgeSegments());
		}

		foldSolutions << solution;
	}
}

double BlockGraph::getTimeLength()
{
	return nbMasters(this) - 1;
}

double BlockGraph::getAvailFoldingVolume()
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QList<QString> mids = availFoldingRegion.keys();

	// trivial case
	if (availFoldingRegion.size() == 1)
	{
		QString mid = mids.front();
		Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[mid]);
		return afr3.area() * masterHeight[mid];
	}

	// AABB of avail folding regions
	QVector<Vector2> conners;
	foreach (QString key, availFoldingRegion.keys())
		conners << availFoldingRegion[key].getConners();
	Geom::Rectangle2 aabb2 = computeAABB2D(conners);

	// pixelize folding region
	Geom::Rectangle aabb3 = base_rect.get3DRectangle(aabb2);
	double pixel_size = aabb3.Extent[0] / 100;
	double pixel_area = pixel_size * pixel_size;
	double AFV = 0;
	foreach (Vector3 gp3, aabb3.getGridSamples(pixel_size))
	{
		Vector2 gp2 = base_rect.getProjCoordinates(gp3);

		double best_height = 0;
		foreach (QString mid, mids)
			if (availFoldingRegion[mid].contains(gp2) && masterHeight[mid] > best_height)
				best_height = masterHeight[mid];

		AFV += best_height;
	}
	AFV *= pixel_area;

	return AFV;
}

Geom::Box BlockGraph::getAvailFoldingSpace( QString mid )
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[mid]);
	Geom::Box afs(afr3, base_rect.Normal, masterHeight[mid]);

	double epsilon = 2 * ZERO_TOLERANCE_LOW;
	afs.Extent += Vector3(epsilon, epsilon, epsilon);

	return afs;
}

void BlockGraph::computeSuperBlock( FdGraph* superKeyframe )
{
	// clone current block
	if (superBlock) delete superBlock;
	superBlock = (FdGraph*)this->clone();

	// offset from keyframe to block
	Vector3 block_pos = baseMaster->center();
	Vector3 keyframe_pos = ((PatchNode*)superKeyframe->getNode(baseMaster->mID))->center();
	Vector3 keyframe2block = block_pos - keyframe_pos;

	// replace parts
	QMap<QString, QString> masterSuperMap = superKeyframe->properties[MASTER_SUPER_MAP].value<QMap<QString, QString> >();
	QMap<QString, QString> M2S; // master-super map in this block
	foreach (PatchNode* m, masters)
	{
		if (masterSuperMap.contains(m->mID))
		{
			// remove master
			superBlock->removeNode(m->mID);

			// clone super patch
			PatchNode* super = (PatchNode*)superKeyframe->getNode(masterSuperMap[m->mID])->clone();
			super->translate(keyframe2block);
			superBlock->Structure::Graph::addNode(super);

			M2S[m->mID] = masterSuperMap[m->mID];
		}
		else
			M2S[m->mID] = m->mID;
	}

	// other stuff
	baseMasterSuper = (PatchNode*)superBlock->getNode(baseMaster->mID);
	foreach (PatchNode* m, masters)
		mastersSuper << (PatchNode*)superBlock->getNode(m->mID);
	foreach (QString mid, masterHeight.keys())
	{
		QString super_mid = M2S[mid];
		masterHeightSuper[super_mid] = masterHeight[mid];
		masterUnderChainsMapSuper[super_mid] = masterUnderChainsMap[mid];
	}
}

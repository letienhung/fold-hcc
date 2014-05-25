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

	// parameters for deformation
	nbSplits = 1;
	nbChunks = 2;

	// thickness
	thickness = 0;
	useThickness = false;

	// selected solution
	selSlnIdx = -1;

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
	minFoldingRegion.clear();
	Geom::Rectangle base_rect = baseMaster->mPatch; // ***use original base rect
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
	maxFoldingRegion.clear();
	Geom::Rectangle base_rect = baseMaster->mPatch;// ***use original base rect
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
		//QVector<Vector3> pnts_proj3;
		//foreach (Vector2 p2, pnts_proj) 
		//	pnts_proj3 << base_rect.getPosition(p2);
		//properties[MAXFR_CP].setValue(pnts_proj3);
	}
}

QVector<QString> BlockGraph::getInbetweenOutsideParts( FdGraph* superKeyframe, QString base_mid, QString top_mid )
{
	// time line
	Vector3 sqzV = baseMaster->mPatch.Normal;
	Geom::Line timeLine(Vector3(0, 0, 0), sqzV);

	// position on time line
	FdNode* base_master = (FdNode*)superBlock->getNode(base_mid);
	FdNode* top_master = (FdNode*)superBlock->getNode(top_mid);
	double t0 = timeLine.getProjTime(base_master->center());
	double t1 = timeLine.getProjTime(top_master->center());
	TimeInterval m1m2 = TIME_INTERVAL(t0, t1);

	// find parts in between m1 and m2
	QVector<QString> inbetweens;
	foreach (FdNode* n, superKeyframe->getFdNodes())
	{
		// skip parts that has been folded
		if (n->hasTag(FOLDED_TAG)) continue;

		// skip parts in this block
		if (superBlock->containsNode(n->mID)) continue;

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

QVector<QString> BlockGraph::getUnrelatedMasters( FdGraph* superKeyframe, QString base_mid, QString top_mid)
{
	QVector<QString> urMasters;

	// retrieve master order constraints
	StringSetMap moc_greater = superKeyframe->properties[MOC_GREATER].value<StringSetMap>();
	StringSetMap moc_less = superKeyframe->properties[MOC_LESS].value<StringSetMap>();

	// related parts with mid1 and mid2
	QSet<QString> base_moc = moc_greater[base_mid] + moc_less[base_mid];
	QSet<QString> top_moc = moc_greater[top_mid] + moc_less[top_mid];	

	// masters unrelated with both
	foreach (Structure::Node* n, superKeyframe->nodes)
	{
		// skip slaves
		if (!n->hasTag(MASTER_TAG)) continue;

		// skip folded masters
		if (n->hasTag(FOLDED_TAG)) continue;

		// accept if not related to any
		if (!base_moc.contains(n->mID) && !top_moc.contains(n->mID))
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
	availFoldingRegion.clear();
	QString base_mid = baseMasterSuper->mID;
	Geom::Rectangle base_rect = baseMaster->mPatch;// ***use original base rect
	foreach (PatchNode* top_master, mastersSuper)
	{
		// skip base master
		QString top_mid = top_master->mID;
		if (top_mid == base_mid) continue;

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

		// avail folding region
		// includes minFR but excludes all constraint points
		Geom::Rectangle2 avail_region = minFoldingRegion[top_mid];
		extendRectangle2D(avail_region, samples_proj);
		availFoldingRegion[top_mid] = avail_region;

		// debug
		QVector<Vector3> samples_proj3;
		foreach (Vector2 p2, samples_proj) 
			samples_proj3 << base_rect.getPosition(p2);
		properties[AFR_CP].setValue(samples_proj3);
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
		// folded chains
		QVector<FdGraph*> foldedChains;
		for (int i = 0; i < chains.size(); i++)
		{
			// skip deleted chain
			if (chains[i]->hasTag(DELETED_TAG))
				foldedChains << NULL;
			else
			{
				double localT = getLocalTime(t, chains[i]->mFoldDuration);
				foldedChains << chains[i]->getKeyframe(localT);
			}
		}

		// combine 
		keyframe = combineDecomposition(foldedChains, baseMaster->mID, masterChainsMap);

		// thickness of masters
		if (useThickness){
			foreach (PatchNode* m, getAllMasters(keyframe))
				m->setThickness(thickness);
		}

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
	// regular key frame w\o thickness
	setUseThickness(false);
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

void BlockGraph::foldabilize(FdGraph* superKeyframe)
{
	// available folding region
	computeAvailFoldingRegion(superKeyframe);
	properties[AFS].setValue(getAFS());

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
		chains[i]->applyFoldOption(fn);

		if (fn)	fn->addTag(SELECTED_TAG);
	}

	addTag(READY_TO_FOLD_TAG);
}

void BlockGraph::filterFoldOptions( QVector<FoldOption*>& options, int cid )
{
	HChain* chain = chains[cid];
	QString top_mid_super = chainTopMasterMapSuper[cid];
	Geom::Rectangle2 AFR = availFoldingRegion[top_mid_super];
	AFR.Extent *= 1.01; // ugly way to avoid numerical issue
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

		// reject if collide with other masters
		// whose time stamp is within the time interval of fn
		if (!reject)
		{
			foreach (QString mid, masterTimeStamps.keys())
			{
				double mstamp = masterTimeStamps[mid];
				if (!within(mstamp, chain->mFoldDuration)) continue;

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
		for (int nS = 1; nS <= nbSplits; nS += 2)
			for (int nUsedChunks = nbChunks; nUsedChunks >= 1; nUsedChunks-- )
				options << chain->generateFoldOptions(nS, nUsedChunks, nbChunks);
		std::cout << "#options = " << options.size();
		filterFoldOptions(options, cid);
		std::cout << " ==> " << options.size() << std::endl;
		foreach(FoldOption* fn, options)
		{
			collFog->addNode(fn);
			collFog->addFoldLink(cn, fn);

			// debug
			frs << base_rect.getProjection(fn->region);
		}
	}

	// debug
	properties[FOLD_REGIONS].setValue(frs);
}

void BlockGraph::addEdgesToCollisionGraph()
{
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
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
	}
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
	QVector<FoldOption*> sln_dummy;
	for (int i = 0; i < chains.size(); i++) sln_dummy << NULL;
	for (int sid = 0; sid < qs.size(); sid++)
	{
		QVector<FoldOption*> solution = sln_dummy;
		foreach(int idx, qs[sid])
		{
			FoldOption* fn = fns[idx];
			ChainNode* cn = collFog->getChainNode(fn->mID);
			solution[cn->chainIdx] = fn;
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
	QList<QString> super_mids = availFoldingRegion.keys();

	// trivial case
	if (availFoldingRegion.size() == 1)
	{
		QString smid = super_mids.front();
		Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[smid]);
		return afr3.area() * masterHeightSuper[smid];
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
		foreach (QString smid, super_mids)
			if (availFoldingRegion[smid].contains(gp2) && masterHeightSuper[smid] > best_height)
				best_height = masterHeightSuper[smid];

		AFV += best_height;
	}
	AFV *= pixel_area;

	return AFV;
}

Geom::Box BlockGraph::getAvailFoldingSpace( QString mid_super )
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle afr3 = base_rect.get3DRectangle(availFoldingRegion[mid_super]);
	Geom::Box afs(afr3, base_rect.Normal, masterHeightSuper[mid_super]);

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
	mastersSuper.clear();
	masterHeightSuper.clear();
	masterUnderChainsMapSuper.clear();
	chainTopMasterMapSuper.clear();

	QString base_mid_super = M2S[baseMaster->mID];
	baseMasterSuper = (PatchNode*)superBlock->getNode(base_mid_super);
	foreach (PatchNode* m, masters)
	{
		QString mid = m->mID;
		QString mid_super = M2S[mid];
		mastersSuper << (PatchNode*)superBlock->getNode(mid_super);
		masterHeightSuper[mid_super] = masterHeight[mid];

		QSet<int> underChainIndices = masterUnderChainsMap[mid];
		masterUnderChainsMapSuper[mid_super] = underChainIndices;
		foreach (int ucid, underChainIndices)
			chainTopMasterMapSuper[ucid] = mid_super;
	}
}

QVector<Geom::Box> BlockGraph::getAFS()
{
	QVector<Geom::Box> afs;
	foreach (QString top_mid_super, availFoldingRegion.keys())
		afs << getAvailFoldingSpace(top_mid_super);
	return afs;
}

void BlockGraph::setNbSplits( int N )
{
	nbSplits = N;
}

void BlockGraph::setNbChunks( int N )
{
	nbChunks = N;
}

void BlockGraph::setThickness( double thk )
{
	thickness = thk;
	if (useThickness)
		updateSolutionWithThickness();
}

void BlockGraph::setUseThickness(bool use)
{
	if (useThickness != use)
	{
		useThickness = use;
		updateSolutionWithThickness();
	}
}

void BlockGraph::updateSolutionWithThickness()
{
	foreach (HChain* chain, chains)
	{
		if (useThickness)
		{
			chain->half_thk = thickness / 2;
			chain->base_offset = thickness / 2;
		}
		else
		{
			chain->half_thk = 0;
			chain->base_offset = 0;
		}
	}

	applySolution(selSlnIdx);
}

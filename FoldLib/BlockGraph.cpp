#include "BlockGraph.h"
#include "FdUtility.h"
#include "IntrRectRect.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "HChain.h"
#include "CliquerAdapter.h"

BlockGraph::BlockGraph( QVector<PatchNode*>& ms, QVector<FdNode*>& ss, 
	QVector< QVector<QString> >& mPairs, QString id )
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
	Geom::Rectangle base_rect = baseMaster->mPatch;
	foreach (PatchNode* top_master, masters)
	{
		// skip base master
		if (top_master == baseMaster) continue;

		// min folding region
		Geom::Rectangle2 min_region = base_rect.get2DRectangle(top_master->mPatch);
		minFoldingRegion[top_master->mID] = min_region;

		// debug
		//foreach (int cid, masterUnderChainsMap[top_master->mID])
			//chains[cid]->addDebugSegments(base_rect.get3DRectangle(min_region).getEdgeSegments());
	}
}

void BlockGraph::computeMaxFoldingRegion(Geom::Box shapeAABB)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	foreach (PatchNode* top_master, masters)
	{
		// skip base master
		if (top_master == baseMaster) continue;

		// 2D points from fold region of slaves and master
		QVector<Vector2> pnts_proj;

		// chains under master
		QVector<Vector3> pnts;
		foreach (int cid, masterUnderChainsMap[top_master->mID]) {
			pnts << chains[cid]->getMaxFoldRegion(true).getConners();
			pnts << chains[cid]->getMaxFoldRegion(false).getConners();
		}
		foreach(Vector3 p, pnts) pnts_proj << base_rect.getProjCoordinates(p);

		// top master
		pnts_proj << minFoldingRegion[top_master->mID].getConners();

		// 2D AABB
		Geom::Rectangle2 max_region = computeAABB2D(pnts_proj);

		// crop by shape AABB
		int aid = shapeAABB.getAxisId(base_rect.Normal);
		Geom::Rectangle cropper3 = shapeAABB.getPatch(aid, 0);
		Geom::Rectangle2 cropper2 = base_rect.get2DRectangle(cropper3);
		max_region.cropByAxisAlignedRectangle(cropper2);

		// max folding region
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

QVector<QString> BlockGraph::getInbetweenOutsideParts( FdGraph* scaffold, QString mid1, QString mid2 )
{
	// time line
	Vector3 sqzV = baseMaster->mPatch.Normal;
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
	foreach (FdNode* n, scaffold->getFdNodes())
	{
		// skip parts that has been folded
		if (n->hasTag(FOLDED_TAG)) continue;

		// skip parts in this block
		if (containsNode(n->mID)) continue;

		// master
		if (n->hasTag(MASTER_TAG))
		{
			double t = timeLine.getProjTime(n->center());

			if (within(t, m1m2)) 
				inbetweens << n->mID;
		}
		// slave
		else
		{
			int aid = n->mBox.getAxisId(sqzV);
			Geom::Segment sklt = n->mBox.getSkeleton(aid);
			double t0 = timeLine.getProjTime(sklt.P0);
			double t1 = timeLine.getProjTime(sklt.P1);
			if (t0 > t1) std::swap(t0, t1);
			TimeInterval ti = TIME_INTERVAL(t0+ZERO_TOLERANCE_LOW, t1-ZERO_TOLERANCE_LOW);

			if (overlap(ti, m1m2))
				inbetweens << n->mID;
		}
	}

	return inbetweens;
}

QVector<QString> BlockGraph::getUnrelatedParts( FdGraph* scaffold, QString mid1, QString mid2, 
	QMultiMap<QString, QString>& moc_greater, QMultiMap<QString, QString>& moc_less )
{
	QVector<QString> urParts;

	QList<QString> moc1, moc2;
	moc1 << moc_greater.values(mid1) << moc_less.values(mid1) << mid1;
	moc2 << moc_greater.values(mid2) << moc_less.values(mid2) << mid2;

	foreach (QString mid, getAllMasterIds(scaffold))
		// no order with both
		if (!moc1.contains(mid) && !moc2.contains(mid))
			urParts << mid;

	return urParts;
}

void BlockGraph::computeAvailFoldingRegion( FdGraph* scaffold, 
	QMultiMap<QString, QString>& moc_greater, QMultiMap<QString, QString>& moc_less )
{
	// compute the offset
	Vector3 posA = baseMaster->center();
	Vector3 posB = ((FdNode*)scaffold->getNode(baseMaster->mID))->center();
	Vector3 offset = posA - posB;

	// align scaffold with this block
	scaffold->translate(offset, false);

	// extent 
	QString base_mid = baseMaster->mID;
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QVector<QString> allMasterIds = getAllMasterIds(scaffold);
	foreach (PatchNode* top_master, masters)
	{
		// skip base master
		if (top_master == baseMaster) continue;
		QString top_mid = top_master->mID;

		// samples from constraint parts: in-between and unordered
		QVector<QString> constraintParts;
		constraintParts << getInbetweenOutsideParts(scaffold, base_mid, top_mid);
		//constraintParts << getUnrelatedParts(scaffold, base_mid, top_mid, moc_greater, moc_less);
		QVector<Vector3> samples;
		int nbs = 10;
		foreach(QString nid, constraintParts)
		{
			FdNode* n = (FdNode*)scaffold->getNode(nid);
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
		double epsilon = 2 * ZERO_TOLERANCE_LOW;
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
	scaffold->translate(-offset, false);
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

FdGraph* BlockGraph::getKeyframeScaffold( double t )
{
	// chains have been created and ready to fold
	if (hasTag(READY_TAG))
	{
		// scaffolds from folded chains
		QVector<FdGraph*> foldedChains;
		for (int i = 0; i < chains.size(); i++)
		{
			double localT = getLocalTime(t, chains[i]->mFoldDuration);
			foldedChains << chains[i]->getKeyframeScaffold(localT);
		}

		// combine 
		FdGraph* keyframeScaffold = combineDecomposition(foldedChains, baseMaster->mID, masterChainsMap);

		// delete folded chains
		foreach (FdGraph* c, foldedChains) delete c;

		return keyframeScaffold;
	}
	// the block is not ready
	// can only answer request on t = 0 and t = 1
	else{
		// clone
		FdGraph* folded = (FdGraph*)this->clone();

		// collapse to base
		Geom::Rectangle base_rect = baseMaster->mPatch;
		if (t > 0.5)
		{
			// this is a ***merged prediction***
			PatchNode* mergedPatch = (PatchNode*)baseMaster->clone();
			mergedPatch->addTag(MERGE_PREDICTION_TAG); 
			//folded->Structure::Graph::addNode(mergedPatch);

			// resize merged patch using available region
			QVector<Vector2> pnts2;
			foreach (QString mid, availFoldingRegion.keys())
				pnts2 << availFoldingRegion[mid].getConners();
			pnts2 << Vector2(-1, -1) << Vector2(1, -1) << Vector2(1, 1) << Vector2(-1, 1);
			Geom::Rectangle2 aabb2 = computeAABB2D(pnts2);
			mergedPatch->resize(aabb2);

			// also keep copy of folded masters
			foreach (FdNode* n, folded->getFdNodes()){
				if (n->hasTag(MASTER_TAG)) {
					// leave base master untouched
					if (n->mID == baseMaster->mID)	continue;

					// move other masters onto base master
					Vector3 c2c = baseMaster->center() - n->center();
					Vector3 up = base_rect.Normal;
					Vector3 offset = dot(c2c, up) * up;
					n->translate(offset);

					// this is a ***folded*** master
					// its role has been taken over by merged prediction patch
					n->addTag(FOLDED_TAG); 
					mergedPatch->appendToContainerProperty<QString>(MERGED_MASTERS_SET, n->mID);
				}else{
					// remove slave nodes
					folded->removeNode(n->mID);
				}
			}
		}

		return folded;
	}

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

	addTag(READY_TAG);
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

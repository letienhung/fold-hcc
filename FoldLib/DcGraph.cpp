#include "DcGraph.h"
#include "PatchNode.h"
#include <QFileInfo>
#include "FdUtility.h"
#include "SectorCylinder.h"
#include "HChain.h"
#include "IntrRect2Rect2.h"


DcGraph::DcGraph(QString id, FdGraph* scaffold, StrArray2D masterGroups, Vector3 v)
	: FdGraph(*scaffold) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;

	// decomposition
	sqzV = v;
	createMasters(masterGroups);
	computeMasterOrderConstraints();
	createSlaves();
	clusterSlaves();
	createBlocks();

	// time scale
	double totalDuration = 0;
	foreach (BlockGraph* b, blocks) totalDuration += b->getTimeLength();
	timeScale = 1.0 / totalDuration;

	// selection
	selBlockIdx = -1;
	keyframeIdx = -1;

	//////////////////////////////////////////////////////////////////////////
	//QVector<Geom::Segment> normals;
	//foreach (PatchNode* m, masters)
	//	normals << Geom::Segment(m->mPatch.Center, m->mPatch.Center + 10 * m->mPatch.Normal);
	//addDebugSegments(normals);
}

DcGraph::~DcGraph()
{
	foreach (BlockGraph* l, blocks)	delete l;
}

void DcGraph::createMasters(StrArray2D& masterGroups)
{
	// merge master groups into patches
	foreach( QVector<QString> masterGroup, masterGroups )
	{
		// merges to a patch or simply returns the single rod
		FdNode* mf = merge(masterGroup);
		QString mf_id = mf->mID;

		// force to change type
		if (mf->mType == FdNode::ROD) 
			changeRodToPatch((RodNode*)mf, sqzV);
		mf = (FdNode*)getNode(mf_id);
		PatchNode* mp = (PatchNode*)mf;

		// consistent normal with sqzV
		if (dot(mp->mPatch.Normal, sqzV) < 0)
			mp->mPatch.flipNormal();

		// tag
		mp->addTag(MASTER_TAG);

		// save
		masters.push_back(mp);
	}
}

void DcGraph::computeMasterOrderConstraints()
{
	// time stamps: bottom-up
	double tScale;
	QMap<QString, double> masterTimeStamps = getTimeStampsNormalized(masters, sqzV, tScale);

	// base master is the bottom one
	double minT = maxDouble();
	foreach(PatchNode* m, masters){
		if (masterTimeStamps[m->mID] < minT){
			baseMaster = m;
			minT = masterTimeStamps[m->mID];
		}
	}

	// projected rect
	QMap<QString, Geom::Rectangle2> proj_rects;
	Geom::Rectangle base_rect = baseMaster->mPatch;
	foreach (PatchNode* m, masters)
		proj_rects[m->mID] = base_rect.get2DRectangle(m->mPatch);

	// check each pair of masters
	for (int i = 0; i < masters.size(); i++){
		for (int j = i+1; j < masters.size(); j++)
		{
			QString mi = masters[i]->mID;
			QString mj = masters[j]->mID;

			// intersect
			if (Geom::IntrRect2Rect2::test(proj_rects[mi], proj_rects[mj]))
			{
				double ti = masterTimeStamps[mi];
				double tj = masterTimeStamps[mj];

				// <up, down>
				if (ti > tj) 
				{
					masterOrderGreater[mi] << mj;
					masterOrderLess[mj] << mi;
				}
				else 
				{
					masterOrderGreater[mj] << mi;
					masterOrderLess[mi] << mj;
				}
					
			}
		}
	}
}

void DcGraph::updateSlaves()
{
	// slave parts
	slaves.clear();
	foreach (FdNode* n, getFdNodes())
		if (!n->hasTag(MASTER_TAG))	slaves << n;

	// debug
	//std::cout << "Slaves:\n";
	//foreach (FdNode* s, slaves) 
	//	std::cout << s->mID.toStdString() << "  ";
}

void DcGraph::updateSlaveMasterRelation()
{
	// initial
	slave2master.clear();
	slave2master.resize(slaves.size());

	// find two masters attaching to a slave
	double adjacentThr = getConnectivityThr();
	for (int i = 0; i < slaves.size(); i++)
	{
		// find adjacent master(s)
		FdNode* slave = slaves[i];
		for (int j = 0; j < masters.size(); j++)
		{
			// distance to master
			PatchNode* master = masters[j];
			if (getDistance(slave, master) < adjacentThr)
			{
				// master id
				slave2master[i] << j;
			}
		}
	}
}

QVector<FdNode*> DcGraph::mergeConnectedCoplanarParts( QVector<FdNode*> ns )
{
	// classify rods and patches
	QVector<RodNode*>	rootRod;
	QVector<PatchNode*> rootPatch;
	foreach (FdNode* n, ns)
	{
		if (n->mType == FdNode::PATCH) 
			rootPatch << (PatchNode*)n;
		else rootRod << (RodNode*)n;
	}

	// RANSAC-like strategy: use a few samples to produce fitting model, which is a plane
	// a plane is fitted by either one root patch or two coplanar root rods
	QVector<Geom::Plane> fitPlanes;
	foreach(PatchNode* pn, rootPatch) fitPlanes << pn->mPatch.getPlane();
	for (int i = 0; i < rootRod.size(); i++){
		for (int j = i+1; j< rootRod.size(); j++)
		{
			Vector3 v1 = rootRod[i]->mRod.Direction;
			Vector3 v2 = rootRod[j]->mRod.Direction;
			double dotProd = dot(v1, v2);
			if (fabs(dotProd) > 0.9)
			{
				Vector3 v = v1 + v2; 
				if (dotProd < 0) v = v1 - v2;
				Vector3 u = rootRod[i]->mRod.Center - rootRod[j]->mRod.Center;
				v.normalize(); u.normalize();
				Vector3 normal = cross(u, v).normalized();
				fitPlanes << Geom::Plane(rootRod[i]->mRod.Center, normal);
			}
		}
	}

	// search for coplanar parts for each fit plane
	// add group them into connected components
	QVector< QSet<QString> > copConnGroups;
	double distThr = getConnectivityThr();
	foreach(Geom::Plane plane, fitPlanes)
	{
		QVector<FdNode*> copNodes;
		foreach(FdNode* n, ns)
			if (onPlane(n, plane)) copNodes << n;

		foreach(QVector<FdNode*> group, getConnectedGroups(copNodes, distThr))
		{
			QSet<QString> groupIds;
			foreach(FdNode* n, group) groupIds << n->mID;
			copConnGroups << groupIds;
		}
	}

	// set cover: prefer large subsets
	QVector<bool> deleted(copConnGroups.size(), false);
	int count = copConnGroups.size(); 
	QVector<int> subsetIndices;
	while(count > 0)
	{
		// search for the largest group
		int maxSize = -1;
		int maxIdx = -1;
		for (int i = 0; i < copConnGroups.size(); i++)
		{
			if (deleted[i]) continue;
			if (copConnGroups[i].size() > maxSize)
			{
				maxSize = copConnGroups[i].size();
				maxIdx = i;
			}
		}

		// save selected subset
		subsetIndices << maxIdx;
		deleted[maxIdx] = true;
		count--;

		// delete overlapping subsets
		for (int i = 0; i < copConnGroups.size(); i++)
		{
			if (deleted[i]) continue;

			QSet<QString> maxGroup = copConnGroups[maxIdx];
			maxGroup.intersect(copConnGroups[i]);
			if (!maxGroup.isEmpty())
			{
				deleted[i] = true;
				count--;
			}
		}
	}

	// merge each selected group
	QVector<FdNode*> mnodes;
	foreach(int i, subsetIndices)
		mnodes << merge(copConnGroups[i].toList().toVector());

	return mnodes;
}

void DcGraph::createSlaves()
{
	// split slave parts by master patches
	double adjacentThr = getConnectivityThr();
	foreach (PatchNode* master, masters)
	{
		foreach(FdNode* n, getFdNodes())
		{
			if (n->hasTag(MASTER_TAG)) continue;

			// split slave if it intersects the master
			if(hasIntersection(n, master, adjacentThr))
				split(n->mID, master->mPatch.getPlane());
		}
	}

	// slaves and slave-master relation
	updateSlaves();
	updateSlaveMasterRelation();

	// connectivity among slave parts
	for (int i = 0; i < slaves.size(); i++)
	{
		for (int j = i+1; j < slaves.size(); j++)
		{
			// skip slave parts connecting to the same master
			if (!(slave2master[i] & slave2master[j]).isEmpty()) 
				continue;

			// add connectivity
			if (getDistance(slaves[i], slaves[j]) < adjacentThr)
			{
				FdGraph::addLink(slaves[i], slaves[j]);
			}
		}
	}

	// merge connected and coplanar slave parts
	foreach (QVector<Structure::Node*> connGroup, getNodesOfConnectedSubgraphs())
	{
		// skip single node
		// two cases: master; T-slave that only connects to master
		if (connGroup.size() == 1) continue;

		// convert to fd Nodes
		QVector<FdNode*> fdConnGroup;
		foreach(Structure::Node* n, connGroup) fdConnGroup << (FdNode*)n;

		mergeConnectedCoplanarParts(fdConnGroup);
	}

	// slave-master relation after merging
	updateSlaves();
	updateSlaveMasterRelation();

	// remove unbounded slaves: unlikely to happen
	for (int i = 0; i < slaves.size();i++)
	{
		if (slave2master[i].size() != 2)
		{
			slaves.remove(i);
			i--;
		}
	}

	// deform each slave slightly so that it attaches to masters perfectly
	for (int i = 0; i < slaves.size(); i++)
		foreach (int mid, slave2master[i])
			slaves[i]->deformToAttach(masters[mid]->mPatch.getPlane());
}

void DcGraph::clusterSlaves()
{
	// clear
	slaveClusters.clear();
	QVector<bool> slaveVisited(slaves.size(), false);

	// build master(V)-slave(E) graph
	Structure::Graph* ms_graph = new Structure::Graph();
	for (int i = 0; i < masters.size(); i++)
	{
		Structure::Node* n = new Structure::Node(QString::number(i));
		ms_graph->addNode(n); // masters are nodes
	}
	for (int i = 0; i < slaves.size(); i++)
	{
		// H-slave are links
		QList<int> mids = slave2master[i].toList();
		QString nj = QString::number(mids.front());
		QString nk = QString::number(mids.last());
		if (ms_graph->getLink(nj, nk) == NULL)
			ms_graph->addLink(nj, nk);
	}

	// enumerate all chordless cycles
	QVector<QVector<QString> > cycle_base = ms_graph->findCycleBase();
	delete ms_graph;

	// collect slaves along each cycle
	QVector<QSet<int> > cycle_slaves;
	foreach (QVector<QString> cycle, cycle_base)
	{
		QSet<int> cs;
		for (int i = 0; i < cycle.size(); i++)
		{
			// master string ids
			QSet<int> mids;
			mids << cycle[i].toInt() << cycle[(i+1)%cycle.size()].toInt();

			// find all siblings
			for (int sid = 0; sid < slaves.size(); sid++){
				if (mids == slave2master[sid])
					cs << sid;
			}
		}
		cycle_slaves << cs;
	}

	// merge cycles who share slaves
	//QMap<int, QSet<int> > merged_cycle_slaves;
	//int count = 0;
	//foreach (QSet<int> cs, cycle_slaves)
	//{
	//	foreach (int key, merged_cycle_slaves.keys())
	//	{
	//		QSet<int> isct = merged_cycle_slaves[key] & cs;
	//		if (!isct.isEmpty()) 
	//		{
	//			// merge and remove old cluster
	//			cs += merged_cycle_slaves[key];
	//			merged_cycle_slaves.remove(key);
	//		}
	//	}

	//	// create a new merged cluster
	//	merged_cycle_slaves[count++] = cs;
	//}

	// save slave clusters 
	//slaveClusters = merged_cycle_slaves.values().toVector();
	mergeIsctSets(cycle_slaves, slaveClusters);
	foreach (QSet<int> cs, slaveClusters)
		foreach(int sid, cs) slaveVisited[sid] = true;

	// edges that are not covered by cycles form individual clusters
	for (int i = 0; i < slaves.size(); i++)
	{
		// skip covered slaves
		if (slaveVisited[i]) continue;

		// create new cluster
		QSet<int> cluster;
		cluster << i;
		slaveVisited[i] = true;

		// find siblings
		for (int sid = 0; sid < slaves.size(); sid++){
			if (!slaveVisited[sid] && slave2master[i] == slave2master[sid])
			{
				cluster << sid;
				slaveVisited[sid] = true;
			}
		}
		slaveClusters << cluster;
	}
}



BlockGraph* DcGraph::createBlock( QSet<int> sCluster )
{
	QVector<PatchNode*> ms;
	QVector<FdNode*> ss;
	QVector< QVector<QString> > mPairs;

	QSet<int> mids; // master indices
	foreach (int sidx, sCluster)
	{
		// slaves
		ss << slaves[sidx]; 

		QVector<QString> mp; 
		foreach (int mid, slave2master[sidx]){
			mids << mid; 
			mp << masters[mid]->mID; 
		}

		// master pairs
		mPairs << mp; 
	}

	// masters
	foreach (int mid, mids)	ms << masters[mid];

	// create
	int bidx = blocks.size();
	QString id = "HB_" + QString::number(bidx);
	Geom::Box aabb = computeAABB().box();
	BlockGraph* b =  new BlockGraph(id, ms, ss, mPairs, aabb);
	blocks << b;

	// master block map
	foreach (PatchNode* m, ms) 
		masterBlockMap[m->mID] << bidx;

	// set up path
	b->path = path;

	return b;
}


void DcGraph::createBlocks()
{
	// clear blocks
	foreach (BlockGraph* b, blocks) delete b;
	blocks.clear();
	masterBlockMap.clear();

	// each H-slave cluster forms a block
	for (int i = 0; i < slaveClusters.size(); i++)
	{
		createBlock(slaveClusters[i]);
	}
}

BlockGraph* DcGraph::getSelBlock()
{
	if (selBlockIdx >= 0 && selBlockIdx < blocks.size())
		return blocks[selBlockIdx];
	else
		return NULL;
}

FdGraph* DcGraph::activeScaffold()
{
	BlockGraph* selLayer = getSelBlock();
	if (selLayer) return selLayer->activeScaffold();
	else		  return this;
}

QStringList DcGraph::getBlockLabels()
{
	QStringList labels;
	foreach(BlockGraph* l, blocks)
		labels.append(l->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

void DcGraph::selectBlock( QString id )
{
	// select layer named id
	selBlockIdx = -1;
	for (int i = 0; i < blocks.size(); i++){
		if (blocks[i]->mID == id){
			selBlockIdx = i;
			break;
		}
	}

	// disable selection on chains
	if (getSelBlock())
		getSelBlock()->selectChain("");
}

FdGraph* DcGraph::getKeyframe( double t )
{
	// folded blocks
	QVector<FdGraph*> foldedBlocks;
	QVector<Geom::Box> activeAFS;
	QVector<Vector3> activeAFR_CP;
	Vector3 activeOrigPosition;
	QString activeBaseMasterID;
	bool showActiveBlockStuff = false;
	for (int i = 0; i < blocks.size(); i++)
	{
		double lt = getLocalTime(t, blocks[i]->mFoldDuration);
		FdGraph* fblock = blocks[i]->getKeyframe(lt);
		foldedBlocks << fblock;

		// debug: active block
		if (lt > 0 && lt < 1)
		{
			activeAFS = blocks[i]->properties[AFS].value<QVector<Geom::Box> >();
			activeAFR_CP = blocks[i]->properties[AFR_CP].value<QVector<Vector3> >();
			activeOrigPosition = blocks[i]->baseMaster->center();
			activeBaseMasterID = blocks[i]->baseMaster->mID;
			showActiveBlockStuff = true;
		}
	}

	// shift layers and add nodes into scaffold
	FdGraph *key_graph = combineDecomposition(foldedBlocks, baseMaster->mID, masterBlockMap);

	// debug
	if (showActiveBlockStuff)
	{
		Vector3 activeCurrPosition = ((PatchNode*)key_graph->getNode(activeBaseMasterID))->center();
		Vector3 offsetV = activeCurrPosition - activeOrigPosition;
		for (int i = 0; i < activeAFS.size(); i++ ) activeAFS[i].translate(offsetV);
		for (int i = 0; i < activeAFR_CP.size(); i++ ) activeAFR_CP[i] += offsetV;
		key_graph->properties[AFS].setValue(activeAFS);
		//key_graph->properties[AFR_CP].setValue(activeAFR_CP);
	}

	// debug
	//addDebugScaffold(key_graph);

	// delete folded blocks
	foreach (FdGraph* b, foldedBlocks) delete b;

	// path
	key_graph->path = path;

	return key_graph;
}

FdGraph* DcGraph::getSuperKeyframe( double t )
{
	// super key frame for each block
	QVector<FdGraph*> foldedBlocks;
	QSet<QString> foldedParts;
	for (int i = 0; i < blocks.size(); i++){
		double lt = getLocalTime(t, blocks[i]->mFoldDuration);
		FdGraph* fblock = blocks[i]->getSuperKeyframe(lt);
		foldedBlocks << fblock;
		// keep track of all folded parts
		foreach (Structure::Node* n, fblock->nodes)
			if (n->hasTag(FOLDED_TAG)) foldedParts << n->mID;
	}

	// combine
	FdGraph *keyframe = combineDecomposition(foldedBlocks, baseMaster->mID, masterBlockMap);
	foreach (FdGraph* b, foldedBlocks) delete b;
	foreach (QString fp, foldedParts)
		keyframe->getNode(fp)->addTag(FOLDED_TAG);

	// super nodes and their children
	QVector<PatchNode*> superPatches;
	QVector<QSet<QString> > childMasters, childMasters_new;
	foreach (PatchNode* m, getAllMasters(keyframe)){
		if (m->hasTag(SUPER_PATCH_TAG)) {
			superPatches << m;
			childMasters << m->properties[MERGED_MASTERS].value<QSet<QString> >();
		}
	}

	// merge super nodes which share children
	QVector<QSet<int> > superIdxClusters = mergeIsctSets(childMasters, childMasters_new);


	// merge to create new super patches
	QMap<QString, QString> masterSuperMap;
	for (int i = 0; i < superIdxClusters.size(); i++)
	{
		// merged set indices
		QList<int> superIndices = superIdxClusters[i].toList();

		// pick up the first
		PatchNode* superPatch_new = superPatches[superIndices[0]];
		superPatch_new->mID = QString("MP_%1").arg(i);
		Geom::Rectangle pred_rect_new = superPatch_new->mPatch;

		// merge with others
		QVector<Vector2> pnts2 = pred_rect_new.get2DConners();
		for (int j = 1; j < superIndices.size(); j++)
		{
			int superIdx = superIndices[j];
			Geom::Rectangle2 rect2 = pred_rect_new.get2DRectangle(superPatches[superIdx]->mPatch);
			pnts2 << rect2.getConners();

			// remove other
			keyframe->removeNode(superPatches[superIdx]->mID);
		}
		Geom::Rectangle2 aabb2 = computeAABB2D(pnts2);
		superPatch_new->resize(aabb2);

		// store master_super_map
		superPatch_new->properties[MERGED_MASTERS].setValue(childMasters_new[i]);
		foreach (QString mid, childMasters_new[i])
			masterSuperMap[mid] = superPatch_new->mID;
	}

	// store master-prediction-map as property
	keyframe->properties[MASTER_SUPER_MAP].setValue(masterSuperMap);

	// update moc_greater
	StringSetMap mocGreater_new = masterOrderGreater;
	// change name of keys
	foreach (QSet<QString> child_mids, childMasters_new)
	{
		QString child = child_mids.toList().front();
		QString key_new = masterSuperMap[child];
		QSet<QString> values_union;
		foreach (QString cmid, child_mids){
			values_union += mocGreater_new[cmid];
			mocGreater_new.remove(cmid);
		}
		mocGreater_new[key_new] = values_union;
	}
	// change name of values
	foreach (QString key, mocGreater_new.keys())
	{
		QSet<QString> values_new;
		foreach (QString v, mocGreater_new[key]){
			if (masterSuperMap.contains(v)) 
				v = masterSuperMap[v]; // change name
			values_new << v;
		}
		mocGreater_new[key] = values_new;
	}

	// moc_less
	StringSetMap mocLess_new;
	foreach (QString key, mocGreater_new.keys())
		foreach(QString value, mocGreater_new[key])
			mocLess_new[value] << key;

	// store as property
	keyframe->properties[MOC_GREATER].setValue(mocGreater_new);
	keyframe->properties[MOC_LESS].setValue(mocLess_new);

	return keyframe;
}

void DcGraph::foldabilize()
{
	// clear tags
	foreach (BlockGraph* b, blocks)
		b->removeTag(READY_TO_FOLD_TAG);

	// initial time intervals
	// greater than 1.0 means never be folded
	foreach (BlockGraph* b, blocks)
	{
		b->mFoldDuration = TIME_INTERVAL(1.0, 2.0);
		b->removeTag(FOLDED_TAG);
	}

	// choose best free block
	std::cout << "\n\n============START============\n";
	double currTime = 0.0;
	FdGraph* currKeyframe = getSuperKeyframe(currTime);
	int next_bid = getBestNextBlockIndex(currTime, currKeyframe);
	while (next_bid >= 0 && next_bid < blocks.size())
	{
		std::cout << "Best next = " << blocks[next_bid]->mID.toStdString() << "\n";

		// foldabilize next block
		BlockGraph* next_block = blocks[next_bid];
		next_block->foldabilize(currKeyframe);
		next_block->addTag(FOLDED_TAG);
		double timeLength = next_block->getTimeLength() * timeScale;
		double nextTime = currTime + timeLength;
		next_block->mFoldDuration = TIME_INTERVAL(currTime, nextTime);

		// get best next
		std::cout << "\n============NEXT============\n";
		currTime = nextTime;
		delete currKeyframe;
		currKeyframe = getSuperKeyframe(currTime);
		next_bid = getBestNextBlockIndex(currTime, currKeyframe);
	}

	// remaining blocks (if any) are interlocking
	QVector<BlockGraph*> blocks_copy = blocks;
	blocks.clear();
	QVector<int> intlkBlockIndices;
	for (int bid = 0; bid < blocks_copy.size(); bid ++)
	{
		BlockGraph* b = blocks_copy[bid];
		if (b->hasTag(FOLDED_TAG)) blocks << b;
		else intlkBlockIndices << bid;
	}

	// merge them as single block to foldabilize
	if (!intlkBlockIndices.isEmpty())
	{
		std::cout << "\n=====MERGE INTERLOCKING======\n";
		QSet<int> sCluster;
		foreach (int bidx, intlkBlockIndices)
			sCluster += slaveClusters[bidx];
		
		BlockGraph* mergedBlock = createBlock(sCluster);
		mergedBlock->foldabilize(currKeyframe);
		mergedBlock->mFoldDuration = TIME_INTERVAL(currTime, 1.0);
	}

	delete currKeyframe;
	std::cout << "\n============FINISH============\n";
}

/**   currT                 nextT                 next2T
	    |------nextBlock------|------next2Block------|
   currKeyframe          nextKeyframe           next2Keyframe
**/  
int DcGraph::getBestNextBlockIndex(double currTime, FdGraph* currKeyframe)
{
	double best_score = -maxDouble();
	int best_next_bid = -1;
	for (int next_bid = 0; next_bid < blocks.size(); next_bid++)
	{
		// skip folded blocks
		BlockGraph* nextBlock = blocks[next_bid];
		if (passed(currTime, nextBlock->mFoldDuration)) continue;

		// predict the folding currBlock
		TimeInterval next_ti = nextBlock->mFoldDuration;
		double timeLength = nextBlock->getTimeLength() * timeScale;
		double nextTime = currTime + timeLength;
		nextBlock->mFoldDuration = TIME_INTERVAL(currTime, nextTime);
		nextBlock->computeAvailFoldingRegion(currKeyframe);
		FdGraph* nextKeyframe = getSuperKeyframe(nextTime);
		// debug
		//keyframes << nextKeyframe;
		//addDebugBoxes(nextBlock->getAFS());
		//addDebugPoints(nextBlock->chains.front()->getTopMaster()->mPatch.getConners());

		// evaluate
		double score = -1;
		if (isValid(nextKeyframe))
		{
			// AFV of nextBlock
			score = nextBlock->getAvailFoldingVolume();

			// AFV of unfolded blocks after nextBlock is folded
			for (int next2_bid = 0; next2_bid < blocks.size(); next2_bid++)
			{
				// skip folded blocks
				BlockGraph* next2Block = blocks[next2_bid];
				if (passed(nextTime, next2Block->mFoldDuration)) continue;

				// predict the folding of next2Block
				TimeInterval next2_ti = next2Block->mFoldDuration;
				double timeLength = next2Block->getTimeLength() * timeScale;
				double next2Time = nextTime + timeLength;
				next2Block->mFoldDuration = TIME_INTERVAL(nextTime, next2Time);
				next2Block->computeAvailFoldingRegion(nextKeyframe);
				FdGraph* next2Keyframe = getSuperKeyframe(next2Time);

				// accumulate score if the folding is valid
				if (isValid(next2Keyframe))
				{
					score += next2Block->getAvailFoldingVolume();
					// debug
					//keyframes << next2Keyframe;
					//addDebugBoxes(next2Block->getAFS());
				}

				// clean up
				//delete next2Keyframe;
				next2Block->mFoldDuration = next2_ti;
			}
		}

		// update the best
		if (score > 0 && score > best_score){
			best_score = score;
			best_next_bid = next_bid;
		}

		// very important: restore time interval
		//delete nextKeyframe;
		nextBlock->mFoldDuration = next_ti;

		// debug
		std::cout << "best score = " << best_score << std::endl;
	}
	return best_next_bid;
}

void DcGraph::generateKeyframes( int N )
{
	keyframes.clear();

	double step = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		keyframes << getKeyframe(i * step);
	}
}

FdGraph* DcGraph::getSelKeyframe()
{
	if (keyframeIdx >= 0 && keyframeIdx < keyframes.size())
		return keyframes[keyframeIdx];
	else return NULL;
}

void DcGraph::exportCollFOG()
{
	BlockGraph* selBlock = getSelBlock();
	if (selBlock) selBlock->exportCollFOG();
}

bool DcGraph::isValid( FdGraph* superKeyframe )
{
	// get all masters: unfolded and super
	QVector<PatchNode*> ms;
	foreach (PatchNode* m, getAllMasters(superKeyframe))
		if (!m->hasTag(FOLDED_TAG))	ms << m;

	// compute time stamps
	double tScale;
	QMap<QString, double> timeStamps = getTimeStampsNormalized(ms, sqzV, tScale);

	// check validity
	StringSetMap moc_greater = superKeyframe->properties[MOC_GREATER].value<StringSetMap>();
	foreach (QString key, moc_greater.keys())
		foreach (QString value, moc_greater[key])
			if (timeStamps[key] < timeStamps[value])
				return false;

	return true;
}

void DcGraph::foldbzSelBlock()
{
	//BlockGraph* selBlock = getSelBlock();
	//if (!selBlock) return;

	//// foldabilize selected block
	//FdGraph* currKeyframe = getKeyframe(0);
	////selBlock->computeAvailFoldingRegion(currKeyframe, masterOrderGreater, masterOrderLess);
	//selBlock->foldabilize();
	//selBlock->mFoldDuration = TIME_INTERVAL(0.0, 1.0);
	//selBlock->addTag(READY_TO_FOLD_TAG);

	//// set unreachable time interval for other blocks
	//TimeInterval ti = TIME_INTERVAL(1.0, 2.0);
	//foreach (BlockGraph* block, blocks)
	//{
	//	if (block != selBlock)
	//		block->mFoldDuration = ti;
	//}
}

void DcGraph::selectKeyframe( int idx )
{
	if (idx >= 0 && idx < keyframes.size())
		keyframeIdx = idx;
}

BlockGraph* DcGraph::mergeBlocks( QVector<BlockGraph*> blocks )
{
	BlockGraph* mergedBlock;

	return mergedBlock;
}
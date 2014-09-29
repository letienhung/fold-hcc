#include "ShapeScaffold.h"
#include "PatchNode.h"
#include <QFileInfo>
#include "FdUtility.h"
#include "SectorCylinder.h"
#include "ChainScaffold.h"
#include "IntrRect2Rect2.h"
#include "TUnitScaffold.h"
#include "HUnitScaffold.h"


ShapeScaffold::ShapeScaffold(QString id, Scaffold* scaffold, Vector3 v, double connThr)
	: Scaffold(*scaffold), baseMaster(nullptr) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;
	sqzV = v;
	// threshold
	connThrRatio = connThr;

	// decompose
	createMasters();
	createSlaves();
	clusterSlaves();
	createBlocks();
	computeBlockWeights();

	// master order constraints
	// to-do: order constraints among all parts
	computeMasterOrderConstraints();

	// time scale
	double totalDuration = 0;
	foreach (UnitScaffold* b, blocks) totalDuration += b->getNbTopMasters();
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

ShapeScaffold::~ShapeScaffold()
{
	foreach (UnitScaffold* l, blocks)	delete l;
}

FdNodeArray2D ShapeScaffold::getPerpConnGroups()
{
	// squeezing direction
	Geom::AABB aabb = computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(sqzV);

	// threshold
	double perpThr = 0.1;
	double connThr = getConnectivityThr();

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<ScaffoldNode*> perpNodes;
	foreach (ScaffoldNode* n, getFdNodes())
	{
		// perp node
		if (n->isPerpTo(sqzV, perpThr))
		{
			perpNodes << n;
		}
		// virtual rod nodes from patch edges
		else if (n->mType == ScaffoldNode::PATCH)
		{
			PatchNode* pn = (PatchNode*)n;
			foreach(RodNode* rn, pn->getEdgeRodNodes())
			{
				// add virtual rod nodes 
				if (rn->isPerpTo(sqzV, perpThr)) 
				{
					Structure::Graph::addNode(rn);
					perpNodes << rn;
					rn->addTag(EDGE_ROD_TAG);
				}
				// delete if not need
				else
				{
					delete rn;
				}
			}
		}
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(sqzAId);
	double perpGroupThr = connThr / skeleton.length();
	QMultiMap<double, ScaffoldNode*> posNodeMap;
	foreach (ScaffoldNode* n, perpNodes){
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}
	FdNodeArray2D perpGroups;
	QList<double> perpPos = posNodeMap.uniqueKeys();
	double pos0 = perpPos.front();
	perpGroups << posNodeMap.values(pos0).toVector();
	for (int i = 1; i < perpPos.size(); i++)
	{
		QVector<ScaffoldNode*> currNodes = posNodeMap.values(perpPos[i]).toVector();
		if (fabs(perpPos[i] - perpPos[i-1]) < perpGroupThr)
			perpGroups.last() += currNodes;	// append to current group
		else perpGroups << currNodes;		// create new group
	}

	// ==STEP 3==: perp & connected groups
	FdNodeArray2D perpConnGroups;
	perpConnGroups << perpGroups.front(); // ground
	for (int i = 1; i < perpGroups.size(); i++){
		foreach(QVector<ScaffoldNode*> connGroup, getConnectedGroups(perpGroups[i], connThr))
			perpConnGroups << connGroup;
	}

	return perpConnGroups;
}

void ShapeScaffold::createMasters()
{
	FdNodeArray2D perpConnGroups = getPerpConnGroups();

	// merge connected groups into patches
	foreach( QVector<ScaffoldNode*> pcGroup,  perpConnGroups)
	{
		// single node
		if (pcGroup.size() == 1)
		{
			ScaffoldNode* n = pcGroup.front();
			if (n->mType == ScaffoldNode::PATCH)	masters << (PatchNode*)n;
			else masters << changeRodToPatch((RodNode*)n, sqzV);
		}
		// multiple nodes
		else
		{
			// check if all nodes in the pcGroup is virtual
			bool allVirtual = true;
			for(ScaffoldNode* pcNode : pcGroup){
				if (!pcNode->hasTag(EDGE_ROD_TAG))
				{
					allVirtual = false;
					break;
				}
			}

			// create bundle master
			if (!allVirtual)
				masters << (PatchNode*)wrapAsBundleNode(getIds(pcGroup), sqzV);
			// each edge rod is converted to a patch master
			else for(ScaffoldNode* pcNode : pcGroup) 
				masters << changeRodToPatch((RodNode*)pcNode, sqzV);
		}
	}

	// normal and tag
	foreach(PatchNode* m, masters)
	{
		// consistent normal with sqzV
		if (dot(m->mPatch.Normal, sqzV) < 0)
			m->mPatch.flipNormal();

		// tag
		m->addTag(MASTER_TAG);
	}
}

void ShapeScaffold::computeMasterOrderConstraints()
{
	// time stamps: bottom-up
	double tScale;
	QMap<QString, double> masterTimeStamps = getTimeStampsNormalized(masters, sqzV, tScale);

	// the base master is the bottom one that is not virtual
	double minT = maxDouble();
	foreach(PatchNode* m, masters){
		if (!m->hasTag(EDGE_ROD_TAG) && masterTimeStamps[m->mID] < minT){
			baseMaster = m;
			minT = masterTimeStamps[m->mID];
		}
	}

	// projected masters on the base master
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

			// test intersection between projections
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

void ShapeScaffold::updateSlaves()
{
	// collect non-master parts as slaves
	slaves.clear();
	foreach (ScaffoldNode* n, getFdNodes())
		if (!n->hasTag(MASTER_TAG))	slaves << n;
}

void ShapeScaffold::updateSlaveMasterRelation()
{
	// initial
	slave2master.clear();
	slave2master.resize(slaves.size());

	// find two masters attaching to a slave
	double adjacentThr = getConnectivityThr();
	for (int i = 0; i < slaves.size(); i++)
	{
		// find adjacent master(s)
		ScaffoldNode* slave = slaves[i];
		for (int j = 0; j < masters.size(); j++)
		{
			PatchNode* master = masters[j];

			// skip if not attached
			if (getDistance(slave, master) > adjacentThr) continue;


			// virtual edge rod master
			if (master->hasTag(EDGE_ROD_TAG))
			{
				// can only attach to its host slave
				QString masterOrig = master->properties[EDGE_ROD_ORIG].value<QString>();
				QString slaveOrig = slave->mID;
				if (slave->properties.contains(SPLIT_ORIG))
					slaveOrig = slave->properties[SPLIT_ORIG].value<QString>();
				if (masterOrig == slaveOrig) slave2master[i] << j;
			}
			// real master
			else
			{
				slave2master[i] << j;
			}
		}
	}
}

void ShapeScaffold::createSlaves()
{
	// split slave parts by master patches
	double connThr = getConnectivityThr();
	for (PatchNode* master : masters) {
		for(ScaffoldNode* n : getFdNodes())
		{
			if (n->hasTag(MASTER_TAG)) continue;
			if(hasIntersection(n, master, connThr))
				split(n->mID, master->mPatch.getPlane());
		}
	}

	// slaves and slave-master relation
	updateSlaves();
	updateSlaveMasterRelation();

	// remove unbounded slaves: unlikely to happen
	int nDeleted = 0;
	for (int i = 0; i < slaves.size();i++)
	{
		if (slave2master[i].isEmpty())
		{
			slaves.remove(i-nDeleted);
			nDeleted++;
		}
	}

	// deform each slave slightly so that it attaches to masters perfectly
	for (int i = 0; i < slaves.size(); i++)
		foreach (int mid, slave2master[i])
			slaves[i]->deformToAttach(masters[mid]->mPatch.getPlane());
}

void ShapeScaffold::clusterSlaves()
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
		QList<int> midPair = slave2master[i].toList();
		if (midPair.size() != 2) continue; // T-slave
		QString nj = QString::number(midPair.front());
		QString nk = QString::number(midPair.last());
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
			// two master ids
			QSet<int> midPair;
			midPair << cycle[i].toInt() << cycle[(i+1)%cycle.size()].toInt();

			// find all siblings: slaves sharing the same pair of masters
			for (int sid = 0; sid < slaves.size(); sid++){
				if (midPair == slave2master[sid])
					cs << sid;
			}
		}
		cycle_slaves << cs;
	}

	// merge cycles who share slaves
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

UnitScaffold* ShapeScaffold::createBlock( QSet<int> sCluster )
{
	QVector<PatchNode*> ms;
	QVector<ScaffoldNode*> ss;
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
	QString id = QString::number(bidx);
	UnitScaffold* b;
	if (ms.size() == 2 && ss.size() == 1 &&
		(ms[0]->hasTag(EDGE_ROD_TAG) || ms[1]->hasTag(EDGE_ROD_TAG)))
		b = new TUnitScaffold("TB_" + id, ms, ss, mPairs);
	else b = new HUnitScaffold("HB_" + id, ms, ss, mPairs);
	b->setAabbConstraint(computeAABB().box());
	blocks << b;

	// master block map
	for (auto m : ms) masterBlockMap[m->mID] << bidx;

	// set up path
	b->path = path;

	return b;
}

void ShapeScaffold::createBlocks()
{
	// clear blocks
	foreach (UnitScaffold* b, blocks) delete b;
	blocks.clear();
	masterBlockMap.clear();

	// each slave cluster forms a block
	for (auto ss : slaveClusters)
	{
		createBlock(ss);
	}
}

UnitScaffold* ShapeScaffold::getSelBlock()
{
	if (selBlockIdx >= 0 && selBlockIdx < blocks.size())
		return blocks[selBlockIdx];
	else
		return NULL;
}

Scaffold* ShapeScaffold::activeScaffold()
{
	UnitScaffold* selLayer = getSelBlock();
	if (selLayer) return selLayer->activeScaffold();
	else		  return this;
}

QStringList ShapeScaffold::getBlockLabels()
{
	QStringList labels;
	foreach(UnitScaffold* l, blocks)
		labels.append(l->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

void ShapeScaffold::selectBlock( QString id )
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

Scaffold* ShapeScaffold::getKeyframe( double t )
{
	bool showActive = false;	
	Vector3 aOrigPos;
	QString aBaseMID;
	QVector<Geom::Box> aAFS;
	QVector<Vector3> aAFR_CP;
	QVector<Vector3> aMaxFR;
	QVector<Geom::Rectangle> aFR; 
	
	// folded blocks
	QVector<Scaffold*> foldedBlocks;
	for (int i = 0; i < blocks.size(); i++)
	{
		double lt = getLocalTime(t, blocks[i]->mFoldDuration);
		Scaffold* fblock = blocks[i]->getKeyframe(lt, true);
		foldedBlocks << fblock;

		// debug: active block
		if (lt > 0 && lt < 1)
		{
			showActive = true;
			aOrigPos = blocks[i]->baseMaster->center();
			aBaseMID = blocks[i]->baseMaster->mID;
			
			// active slaves
			foreach (ScaffoldNode* n, fblock->getFdNodes())
				n->addTag(ACTIVE_NODE_TAG);
		}
	}

	// shift layers and add nodes into scaffold
	Scaffold *key_graph = combineFdGraphs(foldedBlocks, baseMaster->mID, masterBlockMap);

	// delete folded blocks
	foreach (Scaffold* b, foldedBlocks) delete b;

	// in case the combination fails
	if (key_graph == nullptr) return nullptr;

	// path
	key_graph->path = path;

	return key_graph;
}

ShapeSuperKeyframe* ShapeScaffold::getShapeSuperKeyframe( double t )
{
	// super key frame for each block
	QVector<Scaffold*> foldedBlocks;
	QSet<QString> foldedParts;
	for (int i = 0; i < blocks.size(); i++)
	{
		double lt = getLocalTime(t, blocks[i]->mFoldDuration);
		Scaffold* fblock = blocks[i]->getSuperKeyframe(lt);
		foldedBlocks << fblock;

		// nullptr means fblock is unable to fold at time t
		if (fblock == nullptr) return nullptr;
	}

	// combine
	Scaffold *keyframe = combineFdGraphs(foldedBlocks, baseMaster->mID, masterBlockMap);

	// create shape super key frame
	ShapeSuperKeyframe* ssKeyframe = new ShapeSuperKeyframe(keyframe, masterOrderGreater);

	// garbage collection
	delete keyframe;
	for (int i = 0; i < foldedBlocks.size(); i++) delete foldedBlocks[i];

	// return
	return ssKeyframe;
}

void ShapeScaffold::foldabilize()
{
	// clear tags
	foreach (UnitScaffold* b, blocks)
		b->foldabilized = false;

	// initial time intervals: t > 1.0 means not be folded
	foreach (UnitScaffold* b, blocks)
		b->mFoldDuration = INTERVAL(1.0, 2.0);

	// choose best free block
	std::cout << "\n============================="
			  << "\n============START============\n";
	double currTime = 0.0;
	ShapeSuperKeyframe* currKeyframe = getShapeSuperKeyframe(currTime);
	int next_bid = getBestNextBlockIndex(currTime, currKeyframe);  
	std::cout << "Best next = " << next_bid << "\n";

	//return;
	

	while (next_bid >= 0 && next_bid < blocks.size())
	{
		UnitScaffold* next_block = blocks[next_bid];
		std::cout << "Foldabilizing block: " << next_block->mID.toStdString() << "\n";

		// time interval
		double timeLength = next_block->getNbTopMasters() * timeScale;
		double nextTime = currTime + timeLength;

		// foldabilize
		next_block->foldabilizeWrt(currKeyframe);
		next_block->applySolution();
		next_block->mFoldDuration = INTERVAL(currTime, nextTime);

		// get best next
		std::cout << "\n============NEXT============\n";
		currTime = nextTime;
		delete currKeyframe;
		currKeyframe = getShapeSuperKeyframe(currTime);
		next_bid = getBestNextBlockIndex(currTime, currKeyframe);
		std::cout << "Best next = " << next_bid << "\n";
	}

	// remaining blocks (if any) are interlocking
	QVector<UnitScaffold*> blocks_copy = blocks;
	blocks.clear();
	QVector<int> intlkBlockIndices;
	for (int bid = 0; bid < blocks_copy.size(); bid ++)
	{
		UnitScaffold* b = blocks_copy[bid];
		if (b->foldabilized) blocks << b;
		else intlkBlockIndices << bid;
	}

	// merge them as single block to foldabilize
	if (!intlkBlockIndices.isEmpty())
	{
		std::cout << "\n=====MERGE INTERLOCKING======\n";
		QSet<int> sCluster;
		foreach (int bidx, intlkBlockIndices)
			sCluster += slaveClusters[bidx];
		
		UnitScaffold* mergedBlock = createBlock(sCluster);
		mergedBlock->foldabilizeWrt(currKeyframe);
		mergedBlock->applySolution();
		mergedBlock->mFoldDuration = INTERVAL(currTime, 1.0);
	}

	delete currKeyframe;
	std::cout << "\n============FINISH============\n";
}

double ShapeScaffold::foldabilizeBlock(int bid, double currTime, ShapeSuperKeyframe* currKf, 
										double& nextTime, ShapeSuperKeyframe*& nextKf)
{
	// foldabilize nextBlock wrt the current super key frame
	UnitScaffold* nextBlock = blocks[bid];
	double cost = nextBlock->foldabilizeWrt(currKf);
	cost *= blockWeights[bid]; // normalized cost
	nextBlock->applySolution();

	// get the next super key frame 
	double timeLength = nextBlock->getNbTopMasters() * timeScale;
	nextTime = currTime + timeLength;
	nextBlock->mFoldDuration = INTERVAL(currTime, nextTime);
	nextKf = getShapeSuperKeyframe(nextTime);

	// the cost
	return cost;
}

/**   currT                 nextT                 next2T
	    |------nextBlock------|------next2Block------|
   currKeyframe          nextKeyframe           next2Keyframe
**/  
// currKeyframe is a super keyframe providing context information
int ShapeScaffold::getBestNextBlockIndex(double currTime, ShapeSuperKeyframe* currKeyframe)
{
	double min_cost = maxDouble();
	int best_next_bid = -1;
	for (int next_bid = 0; next_bid < blocks.size(); next_bid++)
	{
		UnitScaffold* nextBlock = blocks[next_bid];
		if (passed(currTime, nextBlock->mFoldDuration)) continue; // skip foldabilized blocks

		// foldabilize nextBlock
		double nextTime = -1.0;
		ShapeSuperKeyframe* nextKeyframe = nullptr;
		Interval origNextTi = nextBlock->mFoldDuration; // back up
		double nextCost = foldabilizeBlock(next_bid, currTime, currKeyframe, nextTime, nextKeyframe);

		//nextBlock->showObstaclesAndFoldOptions();
		//return -1;
		
		// the folding of nextBlock must be valid, otherwise skip further evaluation
		if (nextKeyframe->isValid(sqzV))
		{
			// cost of folding remaining open blocks
			for (int next2_bid = 0; next2_bid < blocks.size(); next2_bid++)
			{
				UnitScaffold* next2Block = blocks[next2_bid];
				if (passed(nextTime, next2Block->mFoldDuration)) continue; // skip foldabilized block

				// foldabilize next2Block
				double next2Time;
				ShapeSuperKeyframe* next2Keyframe;
				Interval origNext2Ti = next2Block->mFoldDuration;
				double next2Cost = foldabilizeBlock(next2_bid, nextTime, nextKeyframe, next2Time, next2Keyframe);

				// invalid block folding generates large cost as being deleted
				if (!next2Keyframe->isValid(sqzV)) next2Cost = 1.0; // deletion cost = 1.0

				// update cost : normalized cost
				nextCost += next2Cost * blockWeights[next2_bid];

				delete next2Keyframe; // garbage collection
				next2Block->mFoldDuration = origNext2Ti; // restore time interval
			}

			// nextBlock with lowest cost wins
			if (nextCost < min_cost - ZERO_TOLERANCE_LOW){
				min_cost = nextCost;
				best_next_bid = next_bid;
			}
		}else
		{
			// invalid info for debug
			nextCost = -1;
		}

		delete nextKeyframe; // garbage collection
		nextBlock->mFoldDuration = origNextTi;// restore time interval

		// debug info
		std::cout << blocks[next_bid]->mID.toStdString() << " : " << nextCost << std::endl;
	}

	// found the best next block in terms of introducing most free space for others
	return best_next_bid;
}

void ShapeScaffold::generateKeyframes( int N )
{
	keyframes.clear();

	double step = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		Scaffold* kf = getKeyframe(i * step);
		if(!kf) return;

		keyframes << kf;

		kf->unwrapBundleNodes();
		kf->hideEdgeRods();

		// color
		foreach (ScaffoldNode* n, kf->getFdNodes())
		{
			double grey = 240;
			QColor c = (n->hasTag(ACTIVE_NODE_TAG) && !n->hasTag(MASTER_TAG)) ? 
				QColor::fromRgb(255, 110, 80) : QColor::fromRgb(grey, grey, grey);
			c.setAlphaF(0.78);
			n->mColor = c;
		}
	}
}

Scaffold* ShapeScaffold::getSelKeyframe()
{
	if (keyframeIdx >= 0 && keyframeIdx < keyframes.size())
		return keyframes[keyframeIdx];
	else return NULL;
}

void ShapeScaffold::selectKeyframe( int idx )
{
	if (idx >= 0 && idx < keyframes.size())
		keyframeIdx = idx;
}

UnitScaffold* ShapeScaffold::mergeBlocks( QVector<UnitScaffold*> blocks )
{
    UnitScaffold* mergedBlock = NULL;

	return mergedBlock;
}

double ShapeScaffold::getConnectivityThr()
{
	return connThrRatio * computeAABB().radius();
}

void ShapeScaffold::computeBlockWeights()
{
	blockWeights.clear();

	double totalA = 0;
	for (auto b : blocks)
	{
		double area = b->getChainArea();
		blockWeights << area;
		totalA += area;
	}

	for (auto& bw : blockWeights) bw /= totalA;
}

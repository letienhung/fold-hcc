#include "DcGraph.h"
#include "PatchNode.h"
#include <QFileInfo>
#include "FdUtility.h"
#include "SectorCylinder.h"
#include "TChain.h"


DcGraph::DcGraph(QString id, FdGraph* scaffold, StrArray2D masterGroups, Vector3 v)
	: FdGraph(*scaffold) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;

	// decomposition
	sqzV = v;
	createMasters(masterGroups);
	createSlaves();
	clusterSlaves();
	createBlocks();

	// time scale
	double totalDuration = 0;
	foreach (BlockGraph* b, blocks) totalDuration += nbMasters(b);
	timeScale = 1.0 / totalDuration;

	// selection
	selBlockIdx = -1;
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
		mp->addTag(IS_MASTER);

		// save
		masters.push_back(mp);
	}

	// base master
	baseMasterId = masters.front()->mID;
}

void DcGraph::updateSlaves()
{
	// slave parts
	slaves.clear();
	foreach (FdNode* n, getFdNodes())
		if (!n->hasTag(IS_MASTER))	slaves << n;

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
			if (n->hasTag(IS_MASTER)) continue;

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
	QMap<int, QSet<int> > merged_cycle_slaves;
	int count = 0;
	foreach (QSet<int> cs, cycle_slaves)
	{
		foreach (int key, merged_cycle_slaves.keys())
		{
			QSet<int> isct = merged_cycle_slaves[key] & cs;
			if (!isct.isEmpty()) 
			{
				// merge and remove old cluster
				cs += merged_cycle_slaves[key];
				merged_cycle_slaves.remove(key);
			}
		}

		// create a new merged cluster
		merged_cycle_slaves[count++] = cs;
	}

	// save slave clusters 
	slaveClusters = merged_cycle_slaves.values().toVector();
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

void DcGraph::createBlocks()
{
	// clear blocks
	foreach (BlockGraph* b, blocks) delete b;
	blocks.clear();
	masterBlockMap.clear();

	// each H-slave cluster forms a block
	for (int i = 0; i < slaveClusters.size(); i++)
	{
		QVector<PatchNode*> ms;
		QVector<FdNode*> ss;
		QVector< QVector<QString> > mPairs;

		QSet<int> mids; // master indices
		foreach (int sidx, slaveClusters[i])
		{
			// slaves
			ss << slaves[sidx]; 

			QVector<QString> mp; 
			foreach (int mid, slave2master[sidx])
			{
				mids << mid; 
				mp << masters[mid]->mID; 
			}
			
			// master pairs
			mPairs << mp; 
		}

		// masters
		foreach (int mid, mids)	ms << masters[mid];

		//id
		QString id = "HB_" + QString::number(i);

		// create
		foreach (PatchNode* m, ms) 
			masterBlockMap[m->mID] << blocks.size();
		blocks << new BlockGraph(ms, ss, mPairs, id);
	}

	// set up path
	foreach (BlockGraph* b, blocks)
		b->path = path;
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
	for (int i = 0; i < blocks.size(); i++)
	{
		if (blocks[i]->mID == id)
		{
			selBlockIdx = i;
			break;
		}
	}

	// disable selection on chains
	if (getSelBlock())
	{
		getSelBlock()->selectChain("");
	}
}

FdGraph* DcGraph::getKeyFrame( double t )
{
	// folded blocks
	QVector<FdGraph*> foldedBlocks;
	for (int i = 0; i < blocks.size(); i++)
	{
		double lt = getLocalTime(t, blocks[i]->mFoldDuration);
		foldedBlocks << blocks[i]->getKeyframeScaffold(lt);
	}

	// shift layers and add nodes into scaffold
	FdGraph *key_graph = combineDecomposition(foldedBlocks, baseMasterId, masterBlockMap);

	// delete folded blocks
	foreach (FdGraph* b, foldedBlocks) delete b;

	return key_graph;
}

void DcGraph::foldabilize(bool withinAABB)
{
	Geom::Box aabb = computeAABB().box();
	addDebugBoxes(QVector<Geom::Box>() << aabb);

	// basic folding volumes
	foreach (BlockGraph* b, blocks)
	{
		b->computeMinFoldingVolume();
		b->computeMaxFoldingVolume(aabb);
	}

	// find fold order
	findFoldOrderGreedy();
}

void DcGraph::findFoldOrderGreedy()
{


}

void DcGraph::generateKeyframes( int N )
{
	keyframes.clear();

	double step = 1.0 / (N-1);
	for (int i = 0; i < N; i++)
	{
		keyframes << getKeyFrame(i * step);
	}
}

FdGraph* DcGraph::getSelKeyframe()
{
	if (keyfameIdx >= 0 && keyfameIdx < keyframes.size())
		return keyframes[keyfameIdx];
	else return NULL;
}

void DcGraph::exportCollFOG()
{
	BlockGraph* selBlock = getSelBlock();
	if (selBlock) selBlock->exportCollFOG();
}
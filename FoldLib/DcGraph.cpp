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
	slave2masterSide.clear();
	slave2masterSide.resize(slaves.size());

	// to which side of which master a slave's end is attached
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

				// master side: 2*j (positive) and 2*j+1 (negative)
				Vector3 sc2mc = slave->center() - master->center(); 
				if (dot(sc2mc, master->mPatch.Normal) > 0)
					slave2masterSide[i] << 2*j;
				else
					slave2masterSide[i] << 2*j+1;
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
			QSet<int> s2m_i, s2m_j;
			foreach (int msidx, slave2masterSide[i]) s2m_i << msidx/2;
			foreach (int msidx, slave2masterSide[j]) s2m_j << msidx/2;
			if (!(s2m_i & s2m_j).isEmpty()) continue;

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

	// remove slaves that are still flying: unlikely to happen
	for (int i = 0; i < slaves.size();i++)
	{
		if (slave2masterSide[i].isEmpty())
		{
			slaves.remove(i);
			slave2masterSide.remove(i);
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
	// initial
	TSlaves.clear();
	HSlaveClusters.clear();

	// tags to avoid deleting/copying
	QVector<bool> isVisited(slaves.size(), false);

	// build master (V) slave (E) graph
	Structure::Graph* ms_graph = new Structure::Graph();
	for (int i = 0; i < masters.size(); i++)
	{
		Structure::Node* n = new Structure::Node(QString::number(i));
		ms_graph->addNode(n); // masters are nodes
	}

	for (int i = 0; i < slaves.size(); i++)
	{
		// T-slaves
		if (slave2master[i].size() == 1)
		{
			TSlaves << i;
			isVisited[i] = true;
			continue;
		}

		// H-slave are links
		QList<int> mids = slave2master[i].toList();
		QString nj = QString::number(mids.front());
		QString nk = QString::number(mids.last());

		// for each pair of masters, introduce at most one link between them
		if (ms_graph->getLink(nj, nk) == NULL)
			ms_graph->addLink(nj, nk);
	}

	// enumerate all chordless cycles
	QVector<QVector<QString> > cycle_base = ms_graph->findCycleBase();

	// convert into slave clusters
	QVector<QSet<int> > h_clusters;
	foreach (QVector<QString> cycle, cycle_base)
	{
		// travel tru all edges in the cycle
		QSet<int> cluster;
		for (int i = 0; i < cycle.size(); i++)
		{
			// master string ids
			QString n1 = cycle[i];
			QString n2 = cycle[(i+1)%cycle.size()];

			// add slaves with the same mids to current cluster
			QSet<int> mids;
			mids << n1.toInt() << n2.toInt();
			for (int sid = 0; sid < slaves.size(); sid++){
				if (mids == slave2master[sid])
					cluster << sid;
			}

			// remove edge (n1, n2) from graph
			ms_graph->removeLink(n1, n2);
		}

		// find one cluster
		h_clusters << cluster;
	}

	// merge clusters who share edges
	QMap<int, QSet<int> > merged_clusters;
	int count = 0;
	foreach (QSet<int> hc, h_clusters)
	{
		// merge with all intersected clusters
		foreach (int key, merged_clusters.keys())
		{
			QSet<int> isct = merged_clusters[key] & hc;
			if (!isct.isEmpty()) 
			{
				hc += merged_clusters[key];
				merged_clusters.remove(key);
			}
		}

		// create a new cluster
		merged_clusters[count++] = hc;
	}

	// save H clusters 
	HSlaveClusters = merged_clusters.values().toVector();
	foreach (QSet<int> hc, HSlaveClusters)
		foreach(int sid, hc) isVisited[sid] = true;


	// edges that are not covered by cycles form individual clusters
	foreach (Structure::Link* link, ms_graph->links)
	{
		QSet<int> mids;
		mids << link->nid1.toInt() << link->nid2.toInt();

		QSet<int> cluster;
		for (int sid = 0; sid < slaves.size(); sid++){
			if (!isVisited[sid] && mids == slave2master[sid])
				cluster << sid;
		}

		HSlaveClusters << cluster;
	}

	// delete graph
	delete ms_graph;
}

void DcGraph::createBlocks()
{
	// clear blocks
	foreach (BlockGraph* b, blocks) delete b;
	blocks.clear();
	masterBlockMap.clear();

	// each H-slave cluster forms a block
	for (int i = 0; i < HSlaveClusters.size(); i++)
	{
		QVector<PatchNode*> ms;
		QVector<FdNode*> ss;
		QVector< QVector<QString> > mPairs;

		QSet<int> mids; // master indices
		foreach (int sidx, HSlaveClusters[i])
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
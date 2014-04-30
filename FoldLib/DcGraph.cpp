#include "DcGraph.h"
#include "PatchNode.h"
#include "TBlock.h"
#include "HBlock.h"
#include <QFileInfo>
#include "FdUtility.h"


DcGraph::DcGraph( FdGraph* scaffold, StrArray2D masterGroups, QString id)
	: FdGraph(*scaffold) // clone the FdGraph
{
	path = QFileInfo(path).absolutePath();
	mID = id;

	// merge master groups into patches
	foreach( QVector<QString> panelGroup, masterGroups )
	{
		FdNode* mf = merge(panelGroup);
		mf->addTag(IS_MASTER);

		if (mf->mType != FdNode::PATCH)	changeNodeType(mf);
		masterPatches.push_back((PatchNode*)mf);
	}

	// create blocks
	createBlocks();

	// selection
	selBlockIdx = -1;
}

DcGraph::~DcGraph()
{
	foreach (BlockGraph* l, blocks)	delete l;
}

BlockGraph* DcGraph::getSelLayer()
{
	if (selBlockIdx >= 0 && selBlockIdx < blocks.size())
		return blocks[selBlockIdx];
	else
		return NULL;
}

FdGraph* DcGraph::activeScaffold()
{
	BlockGraph* selLayer = getSelLayer();
	if (selLayer) return selLayer->activeScaffold();
	else		  return this;
}

QStringList DcGraph::getLayerLabels()
{
	QStringList labels;
	foreach(BlockGraph* l, blocks)
		labels.append(l->mID);

	return labels;
}

void DcGraph::selectLayer( QString id )
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
	if (getSelLayer())
	{
		getSelLayer()->selectChain("");
	}
}

FdGraph* DcGraph::getKeyFrame( double t )
{
	//// evenly distribute the time among layers
	//// if the end layer is empty, assign zero time interval to it
	//bool empty_front = (blocks.front()->nbNodes() == 1);
	//bool empty_back = (blocks.back()->nbNodes() == 1);
	//QVector<double> layerStarts;
	//if (empty_front && empty_back)
	//{
	//	layerStarts = getEvenDivision(blocks.size() - 2);
	//	layerStarts.insert(layerStarts.begin(), 0);
	//	layerStarts.append(1);
	//}
	//else if (empty_front)
	//{
	//	layerStarts = getEvenDivision(blocks.size() - 1);
	//	layerStarts.insert(layerStarts.begin(), 0);
	//}
	//else if (empty_back)
	//{
	//	layerStarts = getEvenDivision(blocks.size() - 1);
	//	layerStarts.append(1);
	//}
	//else
	//{
	//	layerStarts = getEvenDivision(blocks.size());
	//}

	//// get folded nodes of each layer
	//// the folding base is the first control panel
	//QVector< QVector<Structure::Node*> > knodes;
	//for (int i = 0; i < blocks.size(); i++)
	//{
	//	double lt = getLocalTime(t, layerStarts[i], layerStarts[i+1]);
	//	knodes << blocks[i]->getKeyFrameNodes(lt);
	//}

	//// compute offset of each control panel
	//// and remove redundant control panels
	//QVector<Vector3> panelDeltas;
	//for (int i = 0; i < masterPatches.size(); i++)
	//{
	//	QString panel_id = masterPatches[i]->mID;
	//	
	//	// copy1 in layer[i]
	//	FdNode* panelCopy1 = NULL;
	//	int idx1 = -1;
	//	QVector<Structure::Node*> &lnodes1 = knodes[i];
	//	for(int j = 0; j < lnodes1.size(); j++)
	//	{
	//		if (lnodes1[j]->hasId(panel_id))
	//		{
	//			panelCopy1 = (FdNode*)lnodes1[j];
	//			idx1 = j;
	//		}
	//	}

	//	// copy2 in layer[i+1]
	//	FdNode* panelCopy2 = NULL;
	//	int idx2 = -1;
	//	QVector<Structure::Node*> &lnodes2 = knodes[i+1];
	//	for(int j = 0; j < lnodes2.size(); j++)
	//	{
	//		if (lnodes2[j]->hasId(panel_id))
	//		{
	//			panelCopy2 = (FdNode*)lnodes2[j];
	//			idx2 = j;
	//		}
	//	}
	//	 
	//	// delta
	//	Vector3 delta(0, 0, 0);
	//	if (panelCopy1 && panelCopy2) 
	//		delta = panelCopy1->center() - panelCopy2->center();
	//	panelDeltas << delta;

	//	// remove copy2
	//	if (idx2 >= 0 && idx2 < lnodes2.size())
	//		lnodes2.remove(idx2);
	//}

	//// shift layers and add nodes into scaffold
	FdGraph *key_graph = new FdGraph();
	//Vector3 offset(0, 0, 0);
	//for (int i = 0; i < knodes.size(); i++)
	//{
	//	// keep the first layer but shift others
	//	if (i > 0) offset += panelDeltas[i-1];

	//	QVector<Structure::Node*> &lnodes = knodes[i];
	//	for (int j = 0; j < lnodes.size(); j++)
	//	{
	//		FdNode* n = (FdNode*)lnodes[j];
	//		n->mBox.translate(offset);
	//		n->createScaffold();

	//		key_graph->Structure::Graph::addNode(n);
	//	}
	//}

	//key_graph->properties["pushAId"] = pushAId;

	return key_graph;
}

void DcGraph::createChains()
{
	// split slave parts by master patches
	foreach (PatchNode* master, masterPatches)
	{
		foreach(FdNode* n, getFdNodes())
		{
			if (n.hasTag(IS_MASTER)) continue;

			// split slave if it intersects the master
			if(hasIntersection(n, master))
				split(n->mID, master->mPatch.getPlane());
		}
	}

	// slave parts
	
	foreach (FdNode* n, getFdNodes())
		if (!n->hasTag(IS_MASTER))
			slaves.push_back(n);

	// attach slave parts onto masters
	slaveSideProp.clear();
	slaveSideProp.resize(slaves.size());
	double adjacentThr = 0.05 * getRadiusOfAABB();
	for (int i = 0; i < slaves.size(); i++)
	{
		FdNode* slave = slaves[i];

		// find adjacent master(s)
		for (int j = 0; j < masterPatches.size(); j++)
		{
			PatchNode* master = masterPatches[j];

			// distance to master
			if (getDistance(slave, master) < adjacentThr)
			{
				// each master j has two sides: 2*j (positive) and 2*j+1 (negative)
				Vector3 sc2mc = slave->center() - master->center(); 
				int side = (dot(sc2mc, master->mPatch.Normal) > 0) ? 2*j : 2*j+1;
				slaveSideProp[i]->insert(side);
			}
		}
	}
}


QVector<FdNode*> DcGraph::mergeCoplanarParts( QVector<FdNode*> ns, PatchNode* panel )
{
	// classify
	QVector<RodNode*>	rootRod;
	QVector<PatchNode*> rootPatch;
	double thr = panel->mBox.getExtent(panel->mPatch.Normal) * 2;
	foreach (FdNode* n, ns)
	{
		if (getDistance(n, panel) < thr)
		{
			if (n->mType == FdNode::PATCH) 
				rootPatch << (PatchNode*)n;
			else rootRod << (RodNode*)n;
		}
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
	QVector< QSet<QString> > copGroups;
	double distThr = computeAABB().radius() * 0.1;
	foreach(Geom::Plane plane, fitPlanes)
	{
		QVector<FdNode*> copNodes;
		foreach(FdNode* n, ns)
			if (onPlane(n, plane)) copNodes << n;

		foreach(QVector<FdNode*> group, clusterNodes(copNodes, distThr))
		{
			QSet<QString> groupIds;
			foreach(FdNode* n, group) groupIds << n->mID;
			copGroups << groupIds;
		}
	}

	// set cover: prefer large subsets
	QVector<bool> deleted(copGroups.size(), false);
	int count = copGroups.size(); 
	QVector<int> subsetIndices;
	while(count > 0)
	{
		// search for the largest group
		int maxSize = -1;
		int maxIdx = -1;
		for (int i = 0; i < copGroups.size(); i++)
		{
			if (deleted[i]) continue;
			if (copGroups[i].size() > maxSize)
			{
				maxSize = copGroups[i].size();
				maxIdx = i;
			}
		}

		// save selected subset
		subsetIndices << maxIdx;
		deleted[maxIdx] = true;
		count--;

		// delete overlapping subsets
		for (int i = 0; i < copGroups.size(); i++)
		{
			if (deleted[i]) continue;

			QSet<QString> maxGroup = copGroups[maxIdx];
			maxGroup.intersect(copGroups[i]);
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
		mnodes << merge(copGroups[i].toList().toVector());

	return mnodes;
}

void DcGraph::createBlocks()
{
	// cluster slaves
	QVector<bool> valid(slaves.size(), true);

	// T-blocks
	QSet<int> T_idx;
	for (int i = 0; i < slaves.size(); i++)
	{
		if (slaveSideProp[i]->size() == 1)
		{
			T_idx.insert(i);
			valid[i] = false;
		}
	}

	// H-blocks
	QList< QSet<int> > sideClusters;
	for (int i = 0; i < slaves.size(); i++)
	{

		if (valid[i])
		{
			for ()
			{
			}
		}
	}



	
	// clear layers
	blocks.clear();

	// barrier box
	Geom::Box bBox = box.scaled(1.01);

	// first layer is pizza
	QString id_first = "Pz-" + QString::number(blocks.size());
	TBlock* pl_first = new TBlock(layerGroups.front(), masterPatches.front(), id_first, bBox);
	pl_first->path = path;
	blocks.push_back(pl_first); 

	// sandwiches
	for (int i = 1; i < layerGroups.size()-1; i++)
	{
		QString id = "Sw-" + QString::number(blocks.size());
		HBlock* sl = new HBlock(layerGroups[i], masterPatches[i-1], masterPatches[i], id, bBox);
		sl->path = path;
		blocks.push_back(sl);
	}
}

void DcGraph::foldabilize()
{
	foreach(BlockGraph* layer, blocks)
		layer->foldabilize();
}
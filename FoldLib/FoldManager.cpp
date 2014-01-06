#include "FoldManager.h"
#include "FdUtility.h"

FoldManager::FoldManager()
{
	scaffold = NULL;

	selId = -1;
	pushAxis = 0;

	keyIndx = -1;
}


FoldManager::~FoldManager()
{
	clearDcGraphs();
}

void FoldManager::clearDcGraphs()
{
	foreach(DcGraph* lm, dcGraphs)
		delete lm;

	dcGraphs.clear();
}


void FoldManager::clearResults()
{
	foreach (QVector<FdGraph*> res, results)
		foreach(FdGraph* g, res) delete g;

	results.clear();
}


DcGraph* FoldManager::getSelDcGraph()
{
	if (selId >= 0 && selId < dcGraphs.size())
		return dcGraphs[selId];
	else
		return NULL;
}

FdGraph* FoldManager::activeScaffold()
{
	DcGraph* selLy = getSelDcGraph();
	if (selLy)	return selLy->activeScaffold();
	else		return scaffold;
}

void FoldManager::setScaffold( FdGraph* fdg )
{
	scaffold = fdg;

	clearDcGraphs();
	updateLists();
}

void FoldManager::foldAlongAxis( int d )
{
	pushAxis = d;
}

void FoldManager::createDcGraphs()
{
	if (!scaffold) return;

	clearDcGraphs();
	clearResults();

	if (pushAxis < 3)
	{
		Vector3 direct(0, 0, 0);
		direct[pushAxis] = 1;;
		createDcGraphs(direct, false);

		qDebug() << "\nvirtual nodes";
		createDcGraphs(direct, true);
	}
	else
	{
		createDcGraphs(Vector3(1,0,0), true);
		createDcGraphs(Vector3(0,1,0), true);
		createDcGraphs(Vector3(0,0,1), true);
	}

	// update ui
	updateLists();
}

void FoldManager::createDcGraphs(Vector3 pushDirect, bool addVirtualPanels)
{
	Geom::AABB aabb = scaffold->computeAABB();
	Geom::Box box = aabb.box();
	int pushAId = aabb.box().getAxisId(pushDirect);

	// add virtual control panels
	if (addVirtualPanels)
	{
		Vector3 upV = box.Axis[pushAId];
		double thk = aabb.radius() / 50;
		MeshPtr emptyMesh(new SurfaceMeshModel());

		Geom::Rectangle topPatch = box.getPatch(pushAId, 1);
		Geom::Box topBox(topPatch, upV, thk);
		FdNode* topVN = scaffold->addNode(emptyMesh, topBox);
		topVN->setStringId("topVirtual");
		topVN->properties["virtual"] = true;

		Geom::Rectangle bottomPatch = box.getPatch(pushAId, -1);
		Geom::Box bottomBox(bottomPatch, -upV, thk);
		FdNode* bottomVN = scaffold->addNode(emptyMesh, bottomBox);
		bottomVN->setStringId("bottomVirtual");
		bottomVN->properties["virtual"] = true;
	}

	// threshold
	double perpThr = 0.1;
	double layerHeightThr = 0.15;
	double clusterDistThr = aabb.radius() * 0.1;
	double areaThr = box.getPatch(pushAId, 0).area() * 0.2;

	// ==STEP 1==: nodes perp to pushing direction
	QVector<FdNode*> perpNodes;
	foreach (FdNode* n, scaffold->getFdNodes())
	{
		if (n->isPerpTo(pushDirect, perpThr))	perpNodes.push_back(n);
	}

	// perp positions
	Geom::Box shapeBox = scaffold->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(pushAId);
	QMultiMap<double, FdNode*> posNodeMap;
	foreach (FdNode* n, perpNodes)
	{
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}

	// ==STEP 2==: group perp nodes
	FdNodeArray2D perpGroups;
	QVector<FdNode*> perpGroup;
	double prePos = 0;
	foreach (double pos, posNodeMap.uniqueKeys())
	{
		// create a new group
		if (fabs(pos - prePos) > layerHeightThr && !perpGroup.isEmpty())
		{
			perpGroups.push_back(perpGroup);
			perpGroup.clear();
		}

		// add to current group
		foreach(FdNode* n, posNodeMap.values(pos))	perpGroup.push_back(n);

		prePos = pos;
	}
	// last group
	if (!perpGroup.isEmpty()) perpGroups.push_back(perpGroup);

	// ==STEP 3==: control panel group
	FdNodeArray2D panelGroups;
	foreach (QVector<FdNode*> pgroup, perpGroups)
	{
		foreach(QVector<FdNode*> cluster, clusterNodes(pgroup, clusterDistThr))
		{
			// accept if size is nontrivial
			Geom::AABB aabb;
			foreach(FdNode* n, cluster)
				aabb.add(n->computeAABB());
			Geom::Box box = aabb.box();
			int aid = box.getAxisId(pushDirect);
			double area = box.getPatch(aid, 0).area();

			if (area > areaThr) panelGroups.push_back(cluster);
		}
	}

	// reject rod structure
	FdNodeArray2D panelGroups2;
	foreach (QVector<FdNode*> panelGroup, panelGroups)
	{
		bool rodStruct = true;
		foreach(FdNode* n, panelGroup)	{
			if (n->mType == FdNode::PATCH){
				rodStruct = false;
				break;
			}
		}

		if (!rodStruct) panelGroups2.push_back(panelGroup);
	}
	
	// ==STEP 4==: create layer models
	// use all control panels
	if (!panelGroups.isEmpty())
	{
		QString id = "Dc-" + QString::number(dcGraphs.size());
		dcGraphs.push_back(new DcGraph(scaffold, getIds(panelGroups), pushDirect, id));
	}

	// exclude rod structures
	if (!panelGroups2.isEmpty() && panelGroups2.size() < panelGroups.size())
	{
		QString id = "Dc-" + QString::number(dcGraphs.size());
		dcGraphs.push_back(new DcGraph(scaffold, getIds(panelGroups2), pushDirect, id));
	}

	// remove virtual nodes
	if (addVirtualPanels)
	{
		scaffold->Structure::Graph::removeNode("topVirtual");
		scaffold->Structure::Graph::removeNode("bottomVirtual");
	}
}

void FoldManager::foldSelLayer()
{
	LayerGraph* lg = getSelLayer();
	if (lg) lg->fold();
}


void FoldManager::snapshotSelLayer( double t )
{
	LayerGraph* lg = getSelLayer();
	if (lg) lg->snapshot(t);

	emit(sceneChanged());
}


void FoldManager::selectDcGraph( QString id )
{
	// selected lyGraph index
	selId = -1;
	for (int i = 0; i < dcGraphs.size(); i++)
	{
		if (dcGraphs[i]->mID == id)
		{	
			selId = i;
			break;
		}
	}

	// disable selection on layer and chains
	selectLayer("");

	// update list
	updateLists();

	// update scene
	emit(sceneChanged());
}

void FoldManager::selectLayer( QString id )
{
	if (getSelDcGraph()) 
	{
		getSelDcGraph()->selectLayer(id);
	}

	updateLists();

	emit(sceneChanged());
}

void FoldManager::selectChain( QString id )
{ 
	if (getSelLayer())
	{
		getSelLayer()->selectChain(id);
	}

	emit(sceneChanged());
}

void FoldManager::updateLists()
{
	QStringList dcGraphLabels;
	QStringList layerLabels;
	QStringList chainLables;
	int nbFrames = 0;

	dcGraphLabels = getDcGraphLabels();
	if (getSelDcGraph()) 
	{
		layerLabels = getSelDcGraph()->getLayerLabels();

		if (getSelLayer())
		{
			chainLables = getSelLayer()->getChainLabels();
		}

		if (selId >= 0 && selId < results.size())
			nbFrames = results[selId].size();
	}

	emit(lyGraphsChanged(dcGraphLabels));
	emit(layersChanged(layerLabels));
	emit(chainsChanged(chainLables));
	emit(keyframesChanged(nbFrames));
}

QStringList FoldManager::getDcGraphLabels()
{
	QStringList labels;
	foreach (DcGraph* ly, dcGraphs)	
		labels.push_back(ly->mID);

	return labels;
}

LayerGraph* FoldManager::getSelLayer()
{
	if (getSelDcGraph())
	{
		return getSelDcGraph()->getSelLayer();
	}
	else
		return NULL;
}

void FoldManager::generateFdKeyFrames()
{
	// clear
	clearResults();

	// generate key frames
	int nbFrames = 100;
	double step = 1.0 / nbFrames;

	// selected dc graph
	foreach (DcGraph* dc_graph, dcGraphs)
	{
		QVector<FdGraph*> dc_results;
		for (int i = 0; i <= nbFrames; i++)
			dc_results << dc_graph->getKeyFrame(i * step);
		results << dc_results;
	}

	emit(resultsGenerated());
	updateLists();
}

void FoldManager::selectKeyframe( int idx )
{
	keyIndx = idx;

	emit(sceneChanged());
}

FdGraph* FoldManager::getKeyframe()
{
	if (selId >= 0 && selId < results.size())
	{
		if (keyIndx >= 0 && keyIndx < results[selId].size())
		{
			return results[selId][keyIndx];
		}
	}
	return NULL;
}

void FoldManager::foldAll()
{
	foreach (DcGraph* dcg, dcGraphs)
		dcg->fold();
}

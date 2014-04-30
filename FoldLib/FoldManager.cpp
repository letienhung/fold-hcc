#include "FoldManager.h"
#include "FdUtility.h"

#include <QFileDialog>
#include <QDir>

FoldManager::FoldManager()
{
	scaffold = NULL;

	selDcIdx = -1;
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

// input
void FoldManager::setScaffold( FdGraph* fdg )
{
	scaffold = fdg;

	clearDcGraphs();
	updateDcList();
}

// masters
void FoldManager::identifyMasters(QString method, QString direct)
{
	if (method == "Parallel")
	{
		Vector3 sqzV(0, 0, 0);
		if (direct == "X") sqzV[0] = 1;
		if (direct == "Y") sqzV[1] = 1;
		if (direct == "Z") sqzV[2] = 1;

		identifyParallelMasters(sqzV);
	}
}

void FoldManager::identifyParallelMasters( Vector3 sqzV )
{
	Geom::AABB aabb = scaffold->computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(sqzV);

	// threshold
	double perpThr = 0.1;
	double blockHeightThr = 0.15;
	double adjacentThr = aabb.radius() * 0.05;
	double areaThr = box.getPatch(sqzAId, 0).area() * 0.2;

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<FdNode*> perpNodes;
	foreach (FdNode* n, scaffold->getFdNodes())
	{
		if (n->isPerpTo(sqzV, perpThr))	perpNodes.push_back(n);
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = scaffold->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(sqzAId);
	QMultiMap<double, FdNode*> posNodeMap;
	foreach (FdNode* n, perpNodes)
	{
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}

	// group
	FdNodeArray2D perpGroups;
	QVector<FdNode*> perpGroup;
	double prePos = 0;
	foreach (double pos, posNodeMap.uniqueKeys())
	{
		// create a new group
		if (fabs(pos - prePos) > blockHeightThr && !perpGroup.isEmpty())
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

	// ==STEP 3==: master groups
	FdNodeArray2D masterGroups;
	foreach (QVector<FdNode*> pgroup, masterGroups)
	{
		// cluster based on adjacency
		foreach(QVector<FdNode*> cluster, clusterNodes(pgroup, adjacentThr))
		{
			// accept if size is nontrivial
			Geom::AABB aabb;
			foreach(FdNode* n, cluster)
				aabb.add(n->computeAABB());
			Geom::Box box = aabb.box();
			int aid = box.getAxisId(sqzV);
			double area = box.getPatch(aid, 0).area();

			if (area > areaThr) masterGroups.push_back(cluster);
		}
	}

	// store ids of master groups
	masterIdGroups = getIds(masterGroups);
}

// decomposition
void FoldManager::decompose()
{
	QString id = "Parallel_Dc";
	dcGraphs.push_back(new DcGraph(scaffold, masterIdGroups, id));
}

void FoldManager::foldbzSelBlock()
{
	BlockGraph* lg = getSelBlock();
	if (lg) lg->foldabilize();
}


void FoldManager::snapshotSelBlock( double t )
{
	BlockGraph* lg = getSelBlock();
	if (lg) lg->snapshot(t);

	emit(sceneChanged());
}

void FoldManager::selectDcGraph( QString id )
{
	// selected lyGraph index
	selDcIdx = -1;
	for (int i = 0; i < dcGraphs.size(); i++)
	{
		if (dcGraphs[i]->mID == id)
		{	
			selDcIdx = i;
			break;
		}
	}

	// disable selection on layer and chains
	selectBlock("");

	// update list
	updateBlockList();
	updateKeyframeList();

	// update scene
	emit(sceneChanged());
}

void FoldManager::selectBlock( QString id )
{
	if (getSelDcGraph()) 
	{
		getSelDcGraph()->selectLayer(id);
	}

	updateChainList();

	emit(sceneChanged());
}

void FoldManager::selectChain( QString id )
{ 
	if (getSelBlock())
	{
		getSelBlock()->selectChain(id);
	}

	emit(sceneChanged());
}

QStringList FoldManager::getDcGraphLabels()
{
	QStringList labels;
	foreach (DcGraph* ly, dcGraphs)	
		labels.push_back(ly->mID);

	return labels;
}

BlockGraph* FoldManager::getSelBlock()
{
	if (getSelDcGraph())
	{
		return getSelDcGraph()->getSelLayer();
	}
	else
		return NULL;
}

void FoldManager::generateKeyframes()
{
	//// clear
	//clearResults();

	//// generate key frames
	//int nbFrames = 10;
	//double step = 1.0 / nbFrames;

	//// selected dc graph
	//foreach (DcGraph* dc_graph, dcGraphs)
	//{
	//	QVector<FdGraph*> dc_results;
	//	for (int i = 0; i <= nbFrames; i++)
	//		dc_results << dc_graph->getKeyFrame(i * step);
	//	results << dc_results;
	//}

	//emit(resultsGenerated());
	//updateKeyframeList();
}

void FoldManager::selectKeyframe( int idx )
{
	//keyfameIdx = idx;

	//emit(sceneChanged());
}

FdGraph* FoldManager::getSelKeyframe()
{
	//if (selDcIdx >= 0 && selDcIdx < results.size())
	//{
	//	if (keyfameIdx >= 0 && keyfameIdx < results[selDcIdx].size())
	//	{
	//		return results[selDcIdx][keyfameIdx];
	//	}
	//}
	return NULL;
}

DcGraph* FoldManager::getSelDcGraph()
{
	if (selDcIdx >= 0 && selDcIdx < dcGraphs.size())
		return dcGraphs[selDcIdx];
	else
		return NULL;
}

FdGraph* FoldManager::activeScaffold()
{
	DcGraph* selLy = getSelDcGraph();
	if (selLy)	return selLy->activeScaffold();
	else		return scaffold;
}

void FoldManager::foldabilize()
{
	foreach (DcGraph* dcg, dcGraphs)
		dcg->foldabilize();
}

void FoldManager::updateDcList()
{
	emit(DcGraphsChanged(getDcGraphLabels()));

	updateBlockList();
	updateKeyframeList();
}

void FoldManager::updateBlockList()
{
	QStringList layerLabels;
	if (getSelDcGraph()) 
		layerLabels = getSelDcGraph()->getLayerLabels();

	emit(blocksChanged(layerLabels));

	updateChainList();
}

void FoldManager::updateChainList()
{
	QStringList chainLables;
	if (getSelBlock())
		chainLables = getSelBlock()->getChainLabels();

	emit(chainsChanged(chainLables));
}


void FoldManager::updateKeyframeList()
{
	//int nbFrames = 0;
	//if (selDcIdx >= 0 && selDcIdx < results.size())
	//	nbFrames = results[selDcIdx].size();

	//emit(keyframesChanged(nbFrames));
}

void FoldManager::exportResultMesh()
{
	//if (results.isEmpty()) return;

	//// Get results output folder
	//QString dataPath = QFileDialog::getExistingDirectory(0, tr("Export Results"), getcwd());
	//// Create folder
	//dataPath += "//" + activeScaffold()->mID;
 //   QDir d(""); d.mkpath(dataPath);

	//for(int i = 0; i < results.size(); i++){
	//	// Create folder
	//	QString currPath = dataPath + "//result"+ QString::number(i+1);
	//	d.mkpath(currPath);
	//	for(int j = 0; j < results[i].size(); j++){
	//		QString filename = currPath + "//" + QString::number(j) + ".obj";
	//		results[i][j]->exportMesh(filename);
	//	}
	//}
}
#include "FoldManager.h"
#include "FdUtility.h"
#include "ChainGraph.h"

#include <QFileDialog>
#include <QDir>

FoldManager::FoldManager()
{
	initScaffold = NULL;
	dcScaffold = NULL;

	selDcIdx = -1;

	sqzV = Vector3(0, 0, 1);

	nbKeyframes = 25;
}

FoldManager::~FoldManager()
{
	clearDcGraphs();
}

void FoldManager::clearDcGraphs()
{
	foreach(DcGraph* dc, dcGraphs)
		delete dc;

	dcGraphs.clear();

	updateDcList();
}

// input
void FoldManager::setScaffold( FdGraph* fdg )
{
	initScaffold = fdg;

	clearDcGraphs();
	updateDcList();
}


void FoldManager::setSqzV( QString sqzV_str )
{
	sqzV = Vector3(0, 0, 0);
	if (sqzV_str == "X") sqzV[0] = 1;
	if (sqzV_str == "Y") sqzV[1] = 1;
	if (sqzV_str == "Z") sqzV[2] = 1;
}


void FoldManager::identifyMasters()
{
	// squeezing direction
	Geom::AABB aabb = initScaffold->computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(sqzV);

	// threshold
	double perpThr = 0.1;
	double masterHeightThr = 0.1;
	double adjacentThr = aabb.radius() * 0.05;

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<FdNode*> perpNodes;
	foreach (FdNode* n, dcScaffold->getFdNodes())
	{
		// perp node
		if (n->isPerpTo(sqzV, perpThr))	
		{
			perpNodes << n;
		}
		// virtual rod nodes from patch edges
		else if (n->mType == FdNode::PATCH)
		{
			PatchNode* pn = (PatchNode*)n;
			foreach(RodNode* rn, pn->getEdgeRodNodes())
			{
				// add virtual rod nodes 
				if (rn->isPerpTo(sqzV, perpThr)) 
				{
					perpNodes << rn;
					dcScaffold->Structure::Graph::addNode(rn);
					rn->addTag(EDGE_ROD_TAG);
				}
				// delete if not need
				else
					delete rn;
			}
		}
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = dcScaffold->computeAABB().box();
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
		if (fabs(pos - prePos) > masterHeightThr && !perpGroup.isEmpty())
		{
			perpGroups << perpGroup;
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
	foreach (QVector<FdNode*> pgroup, perpGroups)
	{
		// cluster based on adjacency
		foreach(QVector<FdNode*> cgroup, getConnectedGroups(pgroup, adjacentThr))
		{
			masterGroups << cgroup;
		}
	}

	// store ids of master groups
	masterIdGroups = getIds(masterGroups);
}

void FoldManager::decompose()
{
	// clone scaffold for composition
	dcScaffold = (FdGraph*)initScaffold->clone();

	// masters
	identifyMasters();

	// decompose
	QString id = "Dc_" + QString::number(dcGraphs.size());
	dcGraphs.push_back(new DcGraph(id, dcScaffold, masterIdGroups, sqzV));

	// delete temp dc scaffold
	delete dcScaffold;

	// update ui
	updateDcList();
}

void FoldManager::foldbzSelBlock()
{
	DcGraph* selDc = getSelDcGraph();
	if (selDc)
		selDc->foldbzSelBlock();
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

	// disable selection on block and chains
	selectBlock("");

	// update list
	updateBlockList();
	updateKeyframeSlider();

	// update scene
	emit(sceneChanged());
}

void FoldManager::selectBlock( QString id )
{
	if (getSelDcGraph()) 
	{
		getSelDcGraph()->selectBlock(id);
	}

	updateChainList();
	updateSolutionList();

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
	foreach (DcGraph* dc, dcGraphs)	
		labels << dc->mID;

	// append string to select none
	labels << "--none--";

	return labels;
}

BlockGraph* FoldManager::getSelBlock()
{
	if (getSelDcGraph())
	{
		return getSelDcGraph()->getSelBlock();
	}
	else
		return NULL;
}

void FoldManager::generateKeyframes()
{
	// selected dc graph
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	// forward message
	selDc->generateKeyframes(nbKeyframes);

	// emit signals
	updateKeyframeSlider();
}

void FoldManager::selectKeyframe( int idx )
{
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	selDc->selectKeyframe(idx);
	emit(sceneChanged());
}

FdGraph* FoldManager::getSelKeyframe()
{
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return NULL;

	return selDc->getSelKeyframe();
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
	else		return initScaffold;
}

void FoldManager::foldabilize()
{
	// decompose
	decompose();
	selDcIdx = 0;
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	// foldabilize
	selDc->foldabilize(); 
	
	//return;

	// forward message
	selDc->generateKeyframes(nbKeyframes);

	// emit signals
	updateKeyframeSlider();
}

void FoldManager::updateDcList()
{
	emit(DcGraphsChanged(getDcGraphLabels()));

	updateBlockList();
	updateKeyframeSlider();
}

void FoldManager::updateBlockList()
{
	QStringList layerLabels;
	if (getSelDcGraph()) 
		layerLabels = getSelDcGraph()->getBlockLabels();

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

void FoldManager::updateKeyframeSlider()
{
	// selected dc graph
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	emit(keyframesChanged(selDc->keyframes.size()-1));
}

void FoldManager::updateSolutionList()
{
	// selected dc graph
	BlockGraph* selBlock = getSelBlock();
	if (!selBlock) return;

	emit(solutionsChanged(selBlock->foldSolutions.size()));
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

void FoldManager::exportCollFOG()
{
	DcGraph* selDc = getSelDcGraph();
	if (selDc)
	{
		selDc->exportCollFOG();
	}
}

void FoldManager::selectSolution( int idx )
{
	// selected dc graph
	BlockGraph* selBlock = getSelBlock();
	if (!selBlock) return;

	selBlock->applySolution(idx);
}

void FoldManager::setNbKeyframes(int N)
{
	nbKeyframes = N;
}

void FoldManager::setNbSplits( int N )
{
	foreach (DcGraph* dc, dcGraphs)
		foreach (BlockGraph* b, dc->blocks)
			b->setNbSplits(N);
}

void FoldManager::setNbChunks( int N )
{
	foreach (DcGraph* dc, dcGraphs)
		foreach (BlockGraph* b, dc->blocks)
			b->setNbChunks(N);
}

void FoldManager::useThickness( int state )
{
	bool use = (state == Qt::Checked);
	foreach (DcGraph* dc, dcGraphs)
		foreach (BlockGraph* b, dc->blocks)
			b->setUseThickness(use);
}

void FoldManager::setThickness( double thk )
{
	foreach (DcGraph* dc, dcGraphs)
		foreach (BlockGraph* b, dc->blocks)
			b->setThickness(thk);
}
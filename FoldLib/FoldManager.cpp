#include "FoldManager.h"
#include "FdUtility.h"
#include "ChainGraph.h"

#include <QFileDialog>
#include <QDir>

FoldManager::FoldManager()
{
	scaffold = NULL;

	selDcIdx = -1;

	sqzV = Vector3(0, 0, 1);

	nbKeyframes = 25;

	nbSplits = 1;
	nbChunks = 2;
	thickness = 0;
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
	scaffold = fdg;

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
	// thickness
	setParameters();

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
	else		return scaffold;
}

void FoldManager::decompose()
{
	selDcIdx = dcGraphs.size();
	QString id = "Dc_" + QString::number(selDcIdx);
	dcGraphs.push_back(new DcGraph(id, scaffold, sqzV));

	updateDcList();
}

void FoldManager::foldabilize()
{
	if (scaffold == NULL)
		return;

	// decompose
	decompose();
	DcGraph* selDc = getSelDcGraph();
	
	// parameters
	setParameters();

	// foldabilize
	selDc->foldabilize(); 
	
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
	nbSplits = N;
	if (!dcGraphs.isEmpty()) setParameters();
}

void FoldManager::setNbChunks( int N )
{
	nbChunks = N;
	if (!dcGraphs.isEmpty()) setParameters();
}

void FoldManager::setThickness( double thk )
{
	thickness = thk;
	if (!dcGraphs.isEmpty()) setParameters();
}

void FoldManager::setParameters()
{
	foreach (DcGraph* dc, dcGraphs)
	foreach (BlockGraph* b, dc->blocks)
	{
		b->setNbSplits(nbSplits);
		b->setNbChunks(nbChunks);
		b->setThickness(thickness);
	}
}

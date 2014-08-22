#include "FoldManager.h"
#include "FdUtility.h"
#include "ChainGraph.h"

#include <QFileDialog>
#include <QDir>

#include <QElapsedTimer>

FoldManager::FoldManager()
{
	scaffold = NULL;

	selDcIdx = -1;

	sqzV = Vector3(0, 0, 1);

	nbKeyframes = 25;

	nbSplits = 1; 
	nbChunks = 2;
	thickness = 0;

	connThrRatio = 0.1;
	aabbScale = Vector3(1, 1, 1.15);

	costWeight = 0.05;
	useNewCost = false;
	useUniformHeight = true;
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
	dcGraphs.push_back(new DcGraph(id, scaffold, sqzV, connThrRatio));

	updateDcList();
}

void FoldManager::foldabilize()
{
	if (scaffold == NULL)
		return;

// start
QElapsedTimer timer;
timer.start();

	// decompose
	decompose();
	DcGraph* selDc = getSelDcGraph();
	
	// parameters
	setParameters();

	// foldabilize
	selDc->foldabilize(); 
	
// timer
int fdTime = timer.elapsed();

	// forward message
	selDc->generateKeyframes(nbKeyframes);

	if(selDc->keyframes.isEmpty()) return;

	// emit signals
	updateKeyframeSlider();

	// save statistics in initial scaffold
	{
		stat.properties[NB_SPLIT] = nbSplits;
		stat.properties[NB_CHUNKS] = nbChunks;
		stat.properties[SQZ_DIRECTION].setValue(sqzV);

		stat.properties[NB_MASTER] = selDc->masters.size();
		stat.properties[NB_SLAVE] = selDc->slaves.size();
		stat.properties[NB_BLOCK] = selDc->blocks.size();

		FdGraph* lastKeyframe = selDc->keyframes.last();
		double origVol = scaffold->computeAABB().box().volume();
		double fdVol = lastKeyframe->computeAABB().box().volume();
		stat.properties[SPACE_SAVING] = 1 - fdVol / origVol;

		stat.properties[FD_TIME] = fdTime;

		int nbHinges = 0;
		double shinkedArea = 0, totalArea = 0;
		foreach(BlockGraph* block, selDc->blocks)
		{
			foreach (ChainGraph* chain, block->chains)
			{
				nbHinges += chain->nbHinges;
				shinkedArea += chain->shrinkedArea;
				totalArea += chain->patchArea;
			}
		}
		foreach (PatchNode* m, selDc->masters) 
			totalArea += m->mPatch.area();

		stat.properties[NB_HINGES] = nbHinges;
		stat.properties[SHRINKED_AREA] = shinkedArea/totalArea;

		stat.properties[CONN_THR_RATIO] = connThrRatio;
		stat.properties[CONSTRAIN_AABB_SCALE].setValue(aabbScale);

		stat.properties[COST_WEIGHT] = costWeight;
	}
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
	Geom::Box constrainAABB = scaffold->computeAABB().box();
	constrainAABB.scale(aabbScale);
	foreach (DcGraph* dc, dcGraphs){
		foreach (BlockGraph* b, dc->blocks)
		{
			b->nbSplits = nbSplits;
			b->nbChunks = nbChunks;
			b->setThickness(thickness);

			b->shapeAABB = constrainAABB;

			b->w = costWeight;

			foreach (ChainGraph* c, b->chains)
				c->useUniformHeight = useUniformHeight;
		}

		dc->connThrRatio = connThrRatio;
	}
}

void FoldManager::setConnThrRatio(double thr)
{
	connThrRatio = thr;
}

void FoldManager::setAabbX( double x )
{
	aabbScale[0] = x;
}

void FoldManager::setAabbY( double y )
{
	aabbScale[1] = y;
}

void FoldManager::setAabbZ( double z )
{
	aabbScale[2] = z;
}

void FoldManager::exportStat()
{
	if (stat.hasTag(FD_TIME))
	{
		QString filename = QFileDialog::getSaveFileName(0, tr("Save Statistics"), NULL, tr("Txt file (*.txt)"));

		QFile file( filename );
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
		QTextStream out(&file);

		Vector3 sqzV = stat.properties[SQZ_DIRECTION].value<Vector3>();
		out << QString("%1 = %2, %3, %4\n").arg(SQZ_DIRECTION).arg(sqzV[0]).arg(sqzV[1]).arg(sqzV[2]);
		out << QString("%1 = %2\n").arg(NB_SPLIT).arg(stat.properties[NB_SPLIT].value<int>());
		out << QString("%1 = %2\n\n").arg(NB_CHUNKS).arg(stat.properties[NB_CHUNKS].value<int>());

		out << QString("%1 = %2\n").arg(NB_MASTER).arg(stat.properties[NB_MASTER].value<int>());
		out << QString("%1 = %2\n").arg(NB_SLAVE).arg(stat.properties[NB_SLAVE].value<int>());
		out << QString("%1 = %2\n\n").arg(NB_BLOCK).arg(stat.properties[NB_BLOCK].value<int>());

		out << QString("%1 = %2\n").arg(FD_TIME).arg(stat.properties[FD_TIME].value<int>());
		out << QString("%1 = %2\n\n").arg(SPACE_SAVING).arg(stat.properties[SPACE_SAVING].value<double>());

		out << QString("%1 = %2\n").arg(NB_HINGES).arg(stat.properties[NB_HINGES].value<int>());
		out << QString("%1 = %2\n\n").arg(SHRINKED_AREA).arg(stat.properties[SHRINKED_AREA].value<double>());

		Vector3 scale = stat.properties[CONSTRAIN_AABB_SCALE].value<Vector3>();
		out << QString("%1 = %2, %3, %4\n").arg(CONSTRAIN_AABB_SCALE).arg(scale[0]).arg(scale[1]).arg(scale[2]);
		out << QString("%1 = %2\n\n").arg(CONN_THR_RATIO).arg(stat.properties[CONN_THR_RATIO].value<double>());

		out << QString("%1 = %2\n").arg(COST_WEIGHT).arg(stat.properties[COST_WEIGHT].value<double>());
	}
}

void FoldManager::setCostWeight( double w )
{
	costWeight = w;
}

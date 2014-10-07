#include "FoldManager.h"
#include "FdUtility.h"
#include "ChainScaff.h"

#include <QFileDialog>
#include <QDir>

#include <QElapsedTimer>

FoldManager::FoldManager()
{
	inputScaffold = nullptr;
	shapeDec = nullptr;

	sqzV = Vector3(0, 0, 1);

	nbKeyframes = 25;

	nbSplits = 1; 
	nbChunks = 2;
	thickness = 0;

	connThrRatio = 0.07;
	aabbScale = Vector3(1, 1, 1);

	costWeight = 0.5;
	useNewCost = false;
}

FoldManager::~FoldManager()
{
	if (shapeDec) delete shapeDec;
}

// input
void FoldManager::setInputScaffold( Scaffold* input )
{
	inputScaffold = input;

	delete shapeDec;
	shapeDec = nullptr;

	updateUnitList();
	updateKeyframeSlider();
}

void FoldManager::setSqzV( QString sqzV_str )
{
	sqzV = Vector3(0, 0, 0);
	if (sqzV_str == "X") sqzV[0] = 1;
	if (sqzV_str == "Y") sqzV[1] = 1;
	if (sqzV_str == "Z") sqzV[2] = 1;
}

void FoldManager::selectUnit( QString id )
{
	if (shapeDec)
		shapeDec->selectUnit(id);

	updateChainList();

	emit(sceneChanged());
}

void FoldManager::selectChain( QString id )
{ 
	if (shapeDec)
		getSelUnit()->selectChain(id);

	emit(sceneChanged());
}

UnitScaff* FoldManager::getSelUnit()
{
	if (shapeDec)
		return shapeDec->getSelUnit();
	else
		return nullptr;
}

void FoldManager::generateKeyframes()
{
	// thickness
	setParameters();

	// selected dec scaffold
	if (!shapeDec) return;

	// forward message
	shapeDec->genKeyframes(nbKeyframes);

	// emit signals
	updateKeyframeSlider();
}

void FoldManager::selectKeyframe( int idx )
{
	if (!shapeDec) return;

	shapeDec->selectKeyframe(idx);
	emit(sceneChanged());
}

Scaffold* FoldManager::getSelKeyframe()
{
	if (!shapeDec) return nullptr;

	return shapeDec->getSelKeyframe();
}

Scaffold* FoldManager::activeScaffold()
{
	DecScaff* as = shapeDec;
	if (as)	return as->activeScaffold();
	else	return inputScaffold;
}

void FoldManager::decompose()
{
	if (shapeDec) delete shapeDec;

	shapeDec = new DecScaff("", inputScaffold, sqzV, connThrRatio);

	updateUnitList();
	updateKeyframeSlider();
}

void FoldManager::foldabilize()
{
	if (inputScaffold == nullptr)
		return;

// start
QElapsedTimer timer;
timer.start();

	// decompose
	decompose();
	
	// parameters
	setParameters();

	// foldabilize
	shapeDec->foldabilize(); 
	
// timer
int fdTime = timer.elapsed();

	// forward message
	shapeDec->genKeyframes(nbKeyframes);

	if (shapeDec->keyframes.isEmpty()) return;

	// emit signals
	updateKeyframeSlider();

	// save statistics in initial scaffold
	{
		stat.properties[NB_SPLIT] = nbSplits;
		stat.properties[NB_CHUNKS] = nbChunks;
		stat.properties[SQZ_DIRECTION].setValue(sqzV);

		stat.properties[NB_MASTER] = shapeDec->masters.size();
		stat.properties[NB_SLAVE] = shapeDec->slaves.size();
		stat.properties[NB_BLOCK] = shapeDec->units.size();

		Scaffold* lastKeyframe = shapeDec->keyframes.last();
		double origVol = inputScaffold->computeAABB().box().volume();
		double fdVol = lastKeyframe->computeAABB().box().volume();
		stat.properties[SPACE_SAVING] = 1 - fdVol / origVol;

		stat.properties[FD_TIME] = fdTime;

		int nbHinges = 0;
		double shinkedArea = 0, totalArea = 0;
		//for(UnitScaff* unit : shapeDec->units)
		//{
		//	for (ChainScaff* chain : unit->chains)
		//	{
		//		nbHinges += chain->nbHinges;
		//		shinkedArea += chain->shrinkedArea;
		//		totalArea += chain->patchArea;
		//	}
		//}
		for (PatchNode* m : shapeDec->masters)
			totalArea += m->mPatch.area();

		stat.properties[NB_HINGES] = nbHinges;
		stat.properties[SHRINKED_AREA] = shinkedArea/totalArea;

		stat.properties[CONN_THR_RATIO] = connThrRatio;
		stat.properties[CONSTRAIN_AABB_SCALE].setValue(aabbScale);

		stat.properties[COST_WEIGHT] = costWeight;
	}
}

void FoldManager::updateUnitList()
{
	QStringList unitLabels;
	if (shapeDec) 
		unitLabels = shapeDec->getUnitLabels();

	emit(unitsChanged(unitLabels));

	updateChainList();
}

void FoldManager::updateChainList()
{
	QStringList chainLables;
	if (getSelUnit())
		chainLables = getSelUnit()->getChainLabels();

	emit(chainsChanged(chainLables));
}

void FoldManager::updateKeyframeSlider()
{
	if (!shapeDec) return;

	emit(keyframesChanged(shapeDec->keyframes.size()-1));
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

void FoldManager::setNbKeyframes(int N)
{
	nbKeyframes = N;
}

void FoldManager::setNbSplits( int N )
{
	nbSplits = N;
	if (shapeDec) setParameters();
}

void FoldManager::setNbChunks( int N )
{
	nbChunks = N;
	if (shapeDec) setParameters();
}

void FoldManager::setThickness( double thk )
{
	thickness = thk;
	if (shapeDec) setParameters();
}

void FoldManager::setParameters()
{
	Geom::Box aabb = inputScaffold->computeAABB().box();
	aabb.scale(aabbScale);

	for(UnitScaff* unit : shapeDec->units)
	{
		unit->setAabbConstraint(aabb);
		unit->maxNbSplits = nbSplits;
		unit->maxNbChunks = nbChunks;
		unit->setThickness(thickness);
		unit->weight = costWeight;

		unit->resetAllFoldOptions();
	}

	shapeDec->connThrRatio = connThrRatio;
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
		QString filename = QFileDialog::getSaveFileName(0, tr("Save Statistics"), nullptr, tr("Txt file (*.txt)"));

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

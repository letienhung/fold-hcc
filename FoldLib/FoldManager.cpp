#include "FoldManager.h"
#include "FdUtility.h"
#include "ChainScaff.h"
#include "ParSingleton.h"

#include <QFileDialog>
#include <QDir>
#include <QElapsedTimer>

FoldManager::FoldManager()
{
	inputScaffold = nullptr;
	shapeDec = nullptr;
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

// selection
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

	emit(keyframesChanged(shapeDec->keyframes.size()));
}

Scaffold* FoldManager::activeScaffold()
{
	if (shapeDec)
		return shapeDec->activeScaffold();
	else	
		return inputScaffold;
}

// foldabilization
void FoldManager::decompose()
{
	if (shapeDec) delete shapeDec;

	// create decomposition
	shapeDec = new DecScaff("", inputScaffold);

	// calculate aabb constraint
	Geom::Box aabb = shapeDec->computeAABB().box();
	aabb.scale(ParSingleton::instance()->aabbCstrScale);
	ParSingleton::instance()->aabbCstr = aabb;

	// update unit list and key frame slider
	updateUnitList();
	updateKeyframeSlider();
}

void FoldManager::generateKeyframes()
{
	// generate
	if (shapeDec)
	{
		shapeDec->genKeyframes();
	}

	// update slider on UI
	updateKeyframeSlider();
}

void FoldManager::foldabilize()
{
	// assert input
	if (!inputScaffold)	return;

	// start timer
	QElapsedTimer timer;
	timer.start();

	// foldabilize
	message("Decomposing...");
	decompose();
	message("Foldabilizing...");
	shapeDec->foldabilize(); 
	
	// end timer
	elapsedTime = timer.elapsed();
	message(QString("Foldabilizing...Done : %1s").arg(elapsedTime / 1000));

	// generate key frames
	message("Generating keyframes...");
	shapeDec->genKeyframes();
	updateKeyframeSlider();
	message("Generating keyframes...Done!");

	// statistics
	//exportStat();
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

void FoldManager::exportStat()
{
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Statistics"), nullptr, tr("Txt file (*.txt)"));

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
	QTextStream out(&file);

	// fold parameters
	ParSingleton* ps = ParSingleton::instance();
	out << QString("%1 = %2, %3, %4\n").arg("sqzDirct").arg(ps->sqzV[0]).arg(ps->sqzV[1]).arg(ps->sqzV[2]);
	out << QString("%1 = %2, %3, %4\n").arg("aabbCstrScale").arg(ps->aabbCstrScale[0]).arg(ps->aabbCstrScale[1]).arg(ps->aabbCstrScale[2]);
	out << QString("%1 = %2\n\n").arg("connThrRatio").arg(ps->connThrRatio);

	out << QString("%1 = %2\n").arg("maxNbSplits").arg(ps->maxNbSplits);
	out << QString("%1 = %2\n\n").arg("maxNbChunks").arg(ps->maxNbChunks);
	out << QString("%1 = %2\n").arg("splitWeight").arg(ps->splitWeight);

	// timing
	out << QString("%1 = %2\n").arg("elapsedTime").arg(elapsedTime);


	// space saving
	Scaffold* lastKeyframe = shapeDec->keyframes.last();
	double origVol = inputScaffold->computeAABB().box().volume();
	double fdVol = lastKeyframe->computeAABB().box().volume();
	double spaceSaving = 1 - fdVol / origVol;
	out << QString("%1 = %2\n\n").arg("spaceSaving").arg(spaceSaving);

	// resulted scaffold
	out << QString("%1 = %2\n").arg("nbMasters").arg(shapeDec->masters.size());
	out << QString("%1 = %2\n").arg("nbSlaves").arg(shapeDec->slaves.size());
	out << QString("%1 = %2\n\n").arg("nbUnits").arg(shapeDec->units.size());

	int nbHinges = 0;
	double shrinkedArea = 0, totalArea = 0;
	for (UnitScaff* unit : shapeDec->units){
		for (ChainScaff* chain : unit->chains)
		{
			//nbHinges += chain->nbHinges;
			//shinkedArea += chain->shrinkedArea;
			//totalArea += chain->patchArea;
		}
	}
	for (PatchNode* m : shapeDec->masters)
		totalArea += m->mPatch.area();
	out << QString("%1 = %2\n").arg("nbHinges").arg(nbHinges);
	out << QString("%1 = %2\n\n").arg("shrinkedArea").arg(shrinkedArea);
}
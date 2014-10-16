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
	aabbCstrScale = Vector3(1, 1, 1);

	nbSplits = 1; 
	nbChunks = 2;
	costWeight = 0.5;

	connThrRatio = 0.07;
	thickness = 0;

	nbKeyframes = 50;
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

// parameters
void FoldManager::setSqzV( QString sqzV_str )
{
	sqzV = Vector3(0, 0, 0);
	if (sqzV_str == "X") sqzV[0] = 1;
	if (sqzV_str == "Y") sqzV[1] = 1;
	if (sqzV_str == "Z") sqzV[2] = 1;
}

void FoldManager::setNbKeyframes(int N)
{
	nbKeyframes = N;
}

void FoldManager::setNbSplits(int N)
{
	nbSplits = N;
	if (shapeDec)
	{
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->maxNbSplits = nbSplits;
		}
	}
}

void FoldManager::setNbChunks(int N)
{
	nbChunks = N;
	if (shapeDec)
	{
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->maxNbChunks = nbChunks;
		}
	}
}

void FoldManager::setThickness(double thk)
{
	thickness = thk;
	if (shapeDec)
	{
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->setThickness(thickness);
		}
	}
}

void FoldManager::setConnThrRatio(double thr)
{
	connThrRatio = thr;
	if (shapeDec)
	{
		shapeDec->connThrRatio = connThrRatio;
	}
}

void FoldManager::setAabbX(double x)
{
	aabbCstrScale[0] = x;
	if (shapeDec)
	{
		Geom::Box box = shapeDec->computeAABB().box();
		box.scale(aabbCstrScale);
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->setAabbCstr(box);
		}
	}
}

void FoldManager::setAabbY(double y)
{
	aabbCstrScale[1] = y;
	if (shapeDec)
	{
		Geom::Box box = shapeDec->computeAABB().box();
		box.scale(aabbCstrScale);
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->setAabbCstr(box);
		}
	}
}

void FoldManager::setAabbZ(double z)
{
	aabbCstrScale[2] = z;
	if (shapeDec)
	{
		Geom::Box box = shapeDec->computeAABB().box();
		box.scale(aabbCstrScale);
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->setAabbCstr(box);
		}
	}
}

void FoldManager::setCostWeight(double w)
{
	costWeight = w;
	if (shapeDec)
	{
		for (UnitScaff* unit : shapeDec->units)
		{
			unit->weight = costWeight;
		}
	}
}

void FoldManager::setAllParameters()
{
	Geom::Box box = shapeDec->computeAABB().box();
	box.scale(aabbCstrScale);

	for (UnitScaff* unit : shapeDec->units)
	{
		unit->setAabbCstr(box);
		unit->maxNbSplits = nbSplits;
		unit->maxNbChunks = nbChunks;
		unit->setThickness(thickness);
		unit->weight = costWeight;
	}

	shapeDec->connThrRatio = connThrRatio;
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

	emit(keyframesChanged(shapeDec->keyframes.size() - 1));
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

	shapeDec = new DecScaff("", inputScaffold, sqzV, connThrRatio);

	updateUnitList();
	updateKeyframeSlider();
}

void FoldManager::generateKeyframes()
{
	// generate
	if (shapeDec)
	{
		setThickness(thickness);
		shapeDec->genKeyframes(nbKeyframes);
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
	setAllParameters();
	message("Foldabilizing...");
	shapeDec->foldabilize(); 
	
	// end timer
	elapsedTime = timer.elapsed();
	message(QString("Foldabilizing...Done : %1s").arg(elapsedTime / 1000));

	// generate key frames
	message("Generating keyframes...");
	shapeDec->genKeyframes(nbKeyframes);
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

	// input
	out << QString("%1 = %2, %3, %4\n").arg("sqzDirct").arg(sqzV[0]).arg(sqzV[1]).arg(sqzV[2]);
	out << QString("%1 = %2, %3, %4\n").arg("aabbCstrScale").arg(aabbCstrScale[0]).arg(aabbCstrScale[1]).arg(aabbCstrScale[2]);
	out << QString("%1 = %2\n\n").arg("connThrRatio").arg(connThrRatio);

	// fold parameters
	out << QString("%1 = %2\n").arg("nbSplits").arg(nbSplits);
	out << QString("%1 = %2\n\n").arg("nbChunks").arg(nbChunks);
	out << QString("%1 = %2\n").arg("costWeight").arg(costWeight);

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
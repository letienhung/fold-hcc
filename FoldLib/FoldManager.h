#pragma once

#include "Scaffold.h"
#include "DecScaffold.h"
#include "Numeric.h"
#include <QObject>

class FoldManager : public QObject
{
	Q_OBJECT

public:
    FoldManager();
	~FoldManager();

public:
	// input
	Scaffold* inputScaffold; 

	// decomposition
	DecScaffold* shapeDec;

	// keyframes
	int nbKeyframes;

	// parameters
	Vector3 sqzV;
	int nbSplits;
	int nbChunks;
	double thickness;
	double connThrRatio;
	Vector3 aabbScale;
	double costWeight;
	bool useNewCost;

	// statistics
	Structure::PropertyMap stat;

public:
	// update Ui
	void updateUnitList();
	void updateChainList();
	void updateKeyframeSlider();

	// getters
	Scaffold* activeScaffold();
	UnitScaffold* getSelUnit();
	Scaffold* getSelKeyframe();

public slots:
	/// Main pipeline	
	// input
	void setInputScaffold(Scaffold* fdg);

	// parameters
	void setSqzV (QString sqzV_str);
	void setNbSplits(int N);
	void setNbChunks(int N);
	void setThickness(double thk);
	void setConnThrRatio(double thr);
	void setAabbX(double x);
	void setAabbY(double y);
	void setAabbZ(double z);
	void setCostWeight(double w);
	void setParameters();

	// decompose
	void decompose();

	// fold
	void foldabilize();

	// keyframes
	void setNbKeyframes(int N);
	void generateKeyframes();

	// output
	void exportResultMesh();
	void exportStat();

	// selection signal from Ui
	void selectUnit(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);

signals:
	// notify others about changes
	void sceneChanged();
	void unitsChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void keyframesChanged(int N);
	void message(QString msg);
};

// statistics
#define NB_SPLIT "nbSplits"
#define NB_CHUNKS "nbChunks"
#define SQZ_DIRECTION "sqzDirection"
#define NB_MASTER "nbMasters"
#define NB_SLAVE "nbSlaves"
#define NB_BLOCK "nbBlocks"
#define NB_HINGES "nbHinges"
#define SHRINKED_AREA "shrinkedArea"
#define FD_TIME "fdTime"
#define SPACE_SAVING "spaceSaving"

#define CONN_THR_RATIO "connThrRatio"
#define CONSTRAIN_AABB_SCALE "constrainAabbScale"
#define COST_WEIGHT "costWeight"

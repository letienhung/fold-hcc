#pragma once

#include "Scaffold.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "UnitScaffold.h"
#include "FoldOptionGraph.h"
#include "ShapeSuperKeyframe.h"

// ShapeGraph encodes the decomposition of scaffold
// including base patches and blocks

class ShapeScaffold : public Scaffold
{
public:
    ShapeScaffold(QString id, Scaffold* scaffold, Vector3 v, double connThr);
	~ShapeScaffold();

public:
	// squeezing direction
	Vector3 sqzV;

	// threshold
	double connThrRatio;

	// masters
	QVector<PatchNode*> masters;

	// slaves
	QVector<ScaffoldNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slaveClusters;

	// blocks
	int selBlockIdx;
	QVector<UnitScaffold*> units;
	QVector<double> unitWeights;

	// more about masters
	PatchNode* baseMaster; // non-virtual master
	QMap<QString, QSet<int> > masterUnitMap;
	QMap<QString, QSet<QString> > masterOrderGreater;
	QMap<QString, QSet<QString> > masterOrderLess;

	// time scale
	double timeScale; // timeScale * block.timeUnits = normalized time

	// folding results
	int keyframeIdx;
	QVector<Scaffold*> keyframes;

private:
	// block 
	void computeUnitWeights();

	// master order constraints
	void computeMasterOrderConstraints();

	// create blocks
	UnitScaffold* createUnit(QSet<int> sCluster);
	void createUnits();

public:
	// threshold for connectivity
	double getConnectivityThr();

	// foldabilization
	void foldabilize();
	double foldabilizeUnit(int bid, double currTime, ShapeSuperKeyframe* currKf, 
									double& nextTime, ShapeSuperKeyframe*& nextKf);
	int getBestNextUnitIdx(double currT, ShapeSuperKeyframe* currKeyframe);

	// key frame
	void generateKeyframes(int N);
	Scaffold* getKeyframe(double t);
	ShapeSuperKeyframe* getShapeSuperKeyframe(double t);

public:
	Scaffold* activeScaffold();
	UnitScaffold* getSelUnit();

	QStringList getUnitLabels();
	void selectUnit(QString id);

	Scaffold* getSelKeyframe();
	void selectKeyframe(int idx);
};


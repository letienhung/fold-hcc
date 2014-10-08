#pragma once

#include "Scaffold.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "UnitScaff.h"
#include "FoldOptGraph.h"
#include "SuperShapeKf.h"

// DecGraph encodes the decomposition of the scaffold for input shape
// including base patches and blocks

class DecScaff final : public Scaffold
{
public:
    DecScaff(QString id, Scaffold* scaffold, Vector3 v, double connThr);
	~DecScaff();

public:
	// squeezing direction
	Vector3 sqzV;

	// threshold
	double connThrRatio;

	// masters
	QVector<PatchNode*> masters;

	// slaves
	QVector<ScaffNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slaveClusters;

	// blocks
	int selBlockIdx;
	QVector<UnitScaff*> units;

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
	void computeUnitImportance();

	// master order constraints
	void computeMasterOrderConstraints();

	// create blocks
	bool areParallel(QVector<ScaffNode*>& ns);
	UnitScaff* createUnit(QSet<int> sCluster);
	void createUnits();

public:
	// threshold for connectivity
	double getConnectThr();

	// foldabilization
	void foldabilize();
	double foldabilizeUnit(UnitScaff* unit, double currTime, SuperShapeKf* currKf,
												double& nextTime, SuperShapeKf*& nextKf);
	UnitScaff* getBestNextUnit(double currT, SuperShapeKf* currKeyframe);

	// key frame
	void genKeyframes(int N);
	Scaffold* getKeyframe(double t);
	SuperShapeKf* getSuperShapeKf(double t);

public:
	Scaffold* activeScaffold();
	UnitScaff* getSelUnit();

	QStringList getUnitLabels();
	void selectUnit(QString id);

	Scaffold* getSelKeyframe();
	void selectKeyframe(int idx);
};


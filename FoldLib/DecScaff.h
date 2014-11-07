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
	// parameters
	Vector3 sqzV;
	Vector3 aabbCstrScale;
	int nbSplits;
	int nbChunks;
	double costWeight;
	double connThrRatio;
	double thickness;

	// masters
	QVector<PatchNode*> masters;

	// slaves
	QVector<ScaffNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slaveClusters;

	// units
	int selUnitIdx;
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
	// unit 
	void computeUnitImportance();

	// master order constraints
	void computeMasterOrderConstraints();

	// create units
	bool areParallel(QVector<ScaffNode*>& ns);
	UnitScaff* createUnit(QSet<int> sCluster);
	void createUnits();

	// interlocking units
	void foldabilizeInterlockUnits(double currTime, SuperShapeKf* currKeyframe);

	// store debug info in key frame
	void storeDebugInfo(Scaffold* kf, int uidx);

public:
	// threshold for connectivity
	double getConnectThr();

	// parameters
	void setParameters();

	// foldabilization
	void foldabilize();
	double foldabilizeUnit(UnitScaff* unit, double currTime, SuperShapeKf* currKf,
												double& nextTime, SuperShapeKf*& nextKf);
	UnitScaff* getBestNextUnit(double currT, SuperShapeKf* currKeyframe);

	// key frame
	void genKeyframes(int N);
	Scaffold* genKeyframe(double t);
	SuperShapeKf* getSuperShapeKf(double t);

public:
	Scaffold* activeScaffold();
	UnitScaff* getSelUnit();

	QStringList getUnitLabels();
	void selectUnit(QString id);

	Scaffold* getSelKeyframe();
	void selectKeyframe(int idx);
};


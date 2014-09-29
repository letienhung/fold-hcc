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

	// masters
	PatchNode* baseMaster; // non-virtual master
	QVector<PatchNode*> masters;
	QMap<QString, QSet<QString> > masterOrderGreater;
	QMap<QString, QSet<QString> > masterOrderLess;

	// slaves
	QVector<ScaffoldNode*> slaves;
	QVector< QSet<int> > slave2master;
	QVector< QSet<int> > slaveClusters;

	// blocks
	int selBlockIdx;
	QVector<UnitScaffold*> blocks;
	QVector<double> blockWeights;
	QMap<QString, QSet<int> > masterBlockMap;

	// time scale
	double timeScale; // timeScale * block.timeUnits = normalized time

	// folding results
	int keyframeIdx;
	QVector<Scaffold*> keyframes;

	// threshold
	double connThrRatio;

public:
	// threshold
	double getConnectivityThr();

	// decomposition
	FdNodeArray2D getPerpConnGroups();
	void computeMasterOrderConstraints();
	void createMasters();

	void updateSlaves();
	void updateSlaveMasterRelation();
	void createSlaves(); 

	void clusterSlaves();
	UnitScaffold* createBlock(QSet<int> sCluster);
	void createBlocks();

	void computeBlockWeights();

	UnitScaffold* mergeBlocks( QVector<UnitScaffold*> blocks );

	// foldabilization
	void foldabilize();
	double foldabilizeBlock(int bid, double currTime, ShapeSuperKeyframe* currKf, 
									double& nextTime, ShapeSuperKeyframe*& nextKf);
	int getBestNextBlockIndex(double currT, ShapeSuperKeyframe* currKeyframe);

	// key frame
	void generateKeyframes(int N);
	Scaffold* getKeyframe(double t);
	ShapeSuperKeyframe* getShapeSuperKeyframe(double t);

public:
	Scaffold* activeScaffold();
	UnitScaffold* getSelBlock();

	QStringList getBlockLabels();
	void selectBlock(QString id);

	Scaffold* getSelKeyframe();
	void selectKeyframe(int idx);
};


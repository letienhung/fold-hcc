#pragma once
#include "FdGraph.h"
#include "Numeric.h"

class LayerModel
{
public:
    LayerModel(FdGraph* scaffold, Vector3 direct);

public:
	int pushAId;
	Vector3 pushDirect;
	FdGraph* layerGraph;

public:
	// build
	void buildUp();

	// control panels
	double perpThr;
	QVector<FdNode*> controlNodes;
	void findControlNodes();

	double layerHeightThr;
	QVector< QVector<FdNode*> > controlGroups;
	void groupControlNodes();

	double areaThr;
	QVector<FdNode*> controlPanels;
	void createControlPanels();

	// layers
	void splitByControlPanels();

	// helpers
	double clusterDistThr;
	QVector< QVector<FdNode*> > clusterNodes(QVector<FdNode*> nodes);
};


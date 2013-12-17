#include "FdGraph.h"


enum DIRECTION{X, Y, Z};

class Foldabilizer
{
public:
    Foldabilizer(FdGraphPtr graph);

public:
	FdGraphPtr scaffold;

public:
	// preparation
	DIRECTION direct;
	void setDirection(DIRECTION i);
	Vector3 getDirection();

	// control panels
	double perpThreshold;
	double layerHeightThreshold;
	QVector<FdNode*> controlNodes;
	QVector< QVector<FdNode*> > controlGroups;
	void findControlNodes();
	void groupControlNodes();
	void createControlPanels();

	double connectThreshold;
	QVector< QVector<FdNode*> > extractConnectedGraphs(QVector<FdNode*> nodes);

	// layers

	// fold a layer

public slots:
	void run();
};


#include "FdGraph.h"


enum DIRECTION{X, Y, Z};

class Foldabilizer
{
public:
    Foldabilizer(FdGraphPtr graph);

public:
	FdGraphPtr scaffold;

	DIRECTION direct;
	double perpThreshold;
	double layerHeightThreshold;
	QVector<FdNode*> controlNodes;
	QVector< QVector<FdNode*> > controlGroups;

public:
	void setDirection(DIRECTION i);
	Vector3 getDirection();
	void findControlNodes();
	void groupControlNodes();
	void createControlPanels();

public slots:
	void run();
};


#include "FdGraph.h"


enum DIRECTION{X, Y, Z};

class Foldabilizer
{
public:
    Foldabilizer(FdGraphPtr graph, Vector3 d);

public:
	FdGraphPtr scaffold;

public:
	// parameter
	int fdAId;
	Vector3 direct;

	// control panels
	double perpThr;
	double layerHeightThr;
	double clusterDistThr;
	double areaThr;
	QVector<FdNode*> controlNodes;
	QVector< QVector<FdNode*> > controlGroups;
	QVector<FdNode*> controlPanels;
	void findControlNodes();
	void groupControlNodes();
	QVector< QVector<FdNode*> > clusterNodes(QVector<FdNode*> nodes);
	void createControlPanels();

	// layers
	void splitByControlPanels();

	// fold a layer

public slots:
	void run();
};


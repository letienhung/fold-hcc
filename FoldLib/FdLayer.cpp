#include "FdLayer.h"

FdLayer::FdLayer( QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id)
{
	this->id = id;
	layer = new FdGraph();

	foreach (FdNode* n, nodes)
	{
		layer->Structure::Graph::addNode(n->clone());
	}

	if (panel1)
	{
		FdNode* cloneP1 = (FdNode*)panel1->clone();
		cloneP1->isCtrlPanel = true;
		layer->Structure::Graph::addNode(cloneP1);

	}

	if (panel2)
	{
		FdNode* cloneP2 = (FdNode*)panel2->clone();
		cloneP2->isCtrlPanel = true;
		layer->Structure::Graph::addNode(cloneP2);
	}
}

FdLayer::~FdLayer()
{
	delete layer;
}

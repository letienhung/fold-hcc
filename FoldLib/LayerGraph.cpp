#include "LayerGraph.h"

LayerGraph::LayerGraph( QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id)
{
	this->id = id;

	foreach (FdNode* n, nodes)
	{
		Structure::Graph::addNode(n->clone());
	}

	if (panel1)
	{
		FdNode* cloneP1 = (FdNode*)panel1->clone();
		cloneP1->isCtrlPanel = true;
		Structure::Graph::addNode(cloneP1);
	}

	if (panel2)
	{
		FdNode* cloneP2 = (FdNode*)panel2->clone();
		cloneP2->isCtrlPanel = true;
		Structure::Graph::addNode(cloneP2);
	}
}

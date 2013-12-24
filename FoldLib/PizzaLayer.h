#pragma once
#include "LayerGraph.h"
#include "DependGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, PatchNode* panel, QString id);
	~PizzaLayer();

	void buildDepGraph();

public: 
	PatchNode* mPanel; 
	DependGraph* dy_graph;
};

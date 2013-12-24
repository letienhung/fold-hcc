#pragma once
#include "LayerGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, PatchNode* panel, QString id);

	void buildDepGraph();

public: 
	PatchNode* mPanel; 
};

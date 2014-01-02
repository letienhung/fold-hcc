#pragma once
#include "LayerGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, PatchNode* panel, QString id);
	~PizzaLayer();

	void buildDependGraph();

public: 
	PatchNode* mPanel; 
};

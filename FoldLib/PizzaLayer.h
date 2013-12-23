#pragma once
#include "LayerGraph.h"
#include "PatchNode.h"


class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, FdNode* panel, QString id);

public: 
	PatchNode* panel; 
};

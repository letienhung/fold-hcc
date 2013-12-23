#pragma once
#include "LayerGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, FdNode* panel, QString id);

public: 
	FdNode* panel; 
};

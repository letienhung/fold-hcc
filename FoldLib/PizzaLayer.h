#pragma once
#include "FdLayer.h"
#include "PatchNode.h"


class PizzaLayer : public FdLayer
{
public:
    PizzaLayer(QVector<FdNode*> nodes, FdNode* panel, QString id);

public: 
	PatchNode* panel; 
};

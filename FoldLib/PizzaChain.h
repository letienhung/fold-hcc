#pragma once

#include "PizzaLayer.h"

class PizzaChain : public PizzaLayer
{
public:
    PizzaChain(QVector<FdNode*> nodes, FdNode* panel, QString id);
};


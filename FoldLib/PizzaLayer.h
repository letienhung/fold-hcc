#pragma once
#include "FdLayer.h"


class PizzaLayer : public FdLayer
{
public:
    PizzaLayer(QVector<FdNode*> nodes, FdNode* panel, QString id);
};

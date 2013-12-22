#pragma once
#include "FdLayer.h"

class SandwichLayer : public FdLayer
{
public:
    SandwichLayer(QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id);
};


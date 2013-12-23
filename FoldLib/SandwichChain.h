#pragma once

#include "SandwichLayer.h"

class SandwichChain : public SandwichLayer
{
public:
    SandwichChain(QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id);
};


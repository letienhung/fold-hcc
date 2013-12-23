#pragma once
#include "LayerGraph.h"

class SandwichLayer : public LayerGraph
{
public:
    SandwichLayer(QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id);
};


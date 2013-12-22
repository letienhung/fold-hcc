#pragma once
#include "FdNode.h"
#include "FdGraph.h"

class FdLayer
{
public:
    FdLayer(QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id);
	~FdLayer();

public:
	FdGraph* layer;
	QString id;
};


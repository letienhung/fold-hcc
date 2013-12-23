#pragma once

#include "FdGraph.h"

class LayerGraph : public FdGraph
{
public:
    LayerGraph(QVector<FdNode*> nodes, FdNode* panel1, FdNode* panel2, QString id);
	~LayerGraph();

	FdGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

public:
	int selId;
	QVector<FdGraph*> chains;
}; 


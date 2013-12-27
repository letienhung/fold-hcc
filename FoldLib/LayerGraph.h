#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "DependGraph.h"

class LayerGraph : public FdGraph
{
public:
	enum LAYER_TYPE{PIZZA, SANDWICH};

public:
    LayerGraph(QVector<FdNode*> nodes, PatchNode* panel1, PatchNode* panel2, QString id);
	~LayerGraph();

	FdGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	virtual void buildDependGraph() = 0;

public:
	LAYER_TYPE mType;
	int selId;
	QVector<FdGraph*> chains;

	DependGraph* dy_graph;
}; 


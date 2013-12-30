#pragma once

#include "ChainGraph.h"

class LayerGraph : public FdGraph
{
public:
	enum LAYER_TYPE{PIZZA, SANDWICH};

public:
    LayerGraph(QVector<FdNode*> nodes, PatchNode* panel1, PatchNode* panel2, QString id);
	~LayerGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	ChainGraph* getChain(QString cid);

	// fold
	virtual void fold();
	virtual void buildDependGraph() = 0;
	void computeChainSequence();

public:
	LAYER_TYPE mType;
	int selId;
	QVector<ChainGraph*> chains;

	DependGraph* dy_graph;
	QVector<QString> chainSequence;
	QVector<FoldingNode*> fnSequence;
}; 


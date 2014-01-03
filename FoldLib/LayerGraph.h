#pragma once

#include "ChainGraph.h"

class LayerGraph : public FdGraph
{
public:
	enum LAYER_TYPE{PIZZA, SANDWICH};

public:
    LayerGraph(QVector<FdNode*> nodes, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox);
	~LayerGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	ChainGraph* getChain(QString cid);

	// fold
	void fold();
	virtual void buildDependGraph() = 0;
	void computeChainSequence();

	// key frames
	virtual QVector<Structure::Node*> getKeyFrameNodes(double t) = 0;

public:
	LAYER_TYPE mType;
	int selId;
	QVector<ChainGraph*> chains;

	DependGraph* dy_graph;
	QVector<QString> chainSequence;
	QVector<FoldingNode*> fnSequence;

	Geom::Box barrierBox;
}; 


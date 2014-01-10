#pragma once

#include "ChainGraph.h"

class LayerGraph : public FdGraph
{
public:
	enum LAYER_TYPE{PIZZA, SANDWICH};

public:
    LayerGraph(QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox);
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

	// depend graph
	virtual void buildDependGraph() = 0;
	virtual void resolveCollision() = 0;

	double computeGain(QString fnid);
	virtual double computeCost(QString fnid) = 0;
	void computeChainSequence();
	void outputDyGraphSequence();

	// key frames
	virtual void snapshot(double t);
	virtual QVector<Structure::Node*> getKeyFrameNodes(double t) = 0;

public:
	LAYER_TYPE mType;
	int selId;
	QVector<ChainGraph*> chains;

	DependGraph* dy_graph;
	QVector<DependGraph*> dygSequence;
	QVector<QString> chainSequence;
	QVector<FoldingNode*> fnSequence;

	Geom::Box barrierBox;
}; 


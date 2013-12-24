#pragma once
#include "Graph.h"
#include "PizzaChain.h"
#include "LayerGraph.h"

enum FD_DIRECTION{FD_LEFT, FD_RIGHT};

class ChainNode : public Structure::Node
{
public:
	ChainNode(int idx, QString id)
		:Node(id), chainIdx(idx){}

	// chain index
	int chainIdx;
};

class FoldingNode : public Structure::Node
{
public:
	FoldingNode(int hidx, FD_DIRECTION d, QString id)
		:Node(id), hingeIdx(hidx), direct(d){}

	// hinge info
	int hingeIdx;
	FD_DIRECTION direct;

	// score
	double score;
};

class DependGraph : public Structure::Graph
{
public:
    DependGraph(LayerGraph* ly);
	DependGraph(DependGraph& other);
	Graph* clone();

	void addFoldingLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	ChainNode* getChainNode(QString id);
	QVector<FoldingNode*> getFoldingNodes(QString id);

public:
	LayerGraph* layer;
};




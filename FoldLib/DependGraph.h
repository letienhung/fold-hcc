#pragma once
#include "Graph.h"

class ChainNode : public Structure::Node
{
public:
	ChainNode(int cIdx, QString id);
	ChainNode(ChainNode &other);
	Node* clone();

	// chain index
	int chainIdx; 
};

class FoldingNode : public Structure::Node
{
public:
	FoldingNode(int hIdx, QString id);
	FoldingNode(FoldingNode &other);
	Node* clone();

	// hinge info
	int hingeIdx;

	// score
	double gain;
	double cost;
	double getScore();
};

class BarrierNode : public Structure::Node
{
public:
	BarrierNode(int fIdx);
	BarrierNode(BarrierNode &other);
	Node* clone();

	// barrier is a face of the bounding box
	int faceIdx;
};

class DependGraph : public Structure::Graph
{
public:
    DependGraph(QString id = "");
	DependGraph(DependGraph& other);
	~DependGraph();
	Graph* clone();

	// modifier
	void addNode(ChainNode* cn);
	void addNode(FoldingNode* fn);
	void addNode(BarrierNode* bn);
	void addFoldingLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	// verifier
	bool verifyNodeType(QString nid, QString type);

	// getters
	ChainNode*		getChainNode(QString fnid);		// chain node of a folding node
	BarrierNode*	getBarrierNode(int fIdx);		// barrier node with face index
	QVector<ChainNode*>		getAllChainNodes();		
	QVector<FoldingNode*>	getAllFoldingNodes();	
	QVector<BarrierNode*>	getAllBarrierNodes();
	QVector<FoldingNode*>	getSiblings(QString fnid);		// siblings of folding node
	QVector<FoldingNode*>	getFoldingNodes(QString cnid);	// folding nodes of a chain node
	QVector<Structure::Node*> getFamilyNodes(QString nid);
	QVector<Structure::Link*> getFoldinglinks(QString nid);
	QVector<Structure::Link*> getCollisionLinks(QString nid);
	QVector<Structure::Link*> getFamilyCollisionLinks(QString nid);

	// fold
	bool isFreeChainNode(QString cnid);
    FoldingNode* getBestFoldingNode();

	// visualize
	QString toGraphvizFormat(QString subcaption, QString caption);
	void saveAsGraphviz(QString fname, QString subcaption = "", QString caption = "");
	void saveAsImage(QString fname);

public:
	static QString dotPath;
};
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
	FoldingNode(int hIdx, bool right, double s, double p, int n, QString id);
	FoldingNode(FoldingNode &other);
	Node* clone();

	// hinge info: idx of root segment
	int hingeIdx;

	// to which side to fold
	bool rightSide;

	// deformation
	double scale;
	double position;

	// #splits
	int nbsplit;

	// cost
	double getCost();

	// printable info
	QString getInfo();
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


class FoldOptionGraph : public Structure::Graph
{
public:
	FoldOptionGraph(QString id = "");
	FoldOptionGraph(FoldOptionGraph& other);
	~FoldOptionGraph();
	Graph* clone();

	// modifier
	void addNode(ChainNode* cn);
	void addNode(FoldingNode* fn);
	void addNode(BarrierNode* bn);
	void addFoldingLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	// verifier
	bool verifyNodeType(QString nid, QString type);
	bool verifyLinkType(QString nid1, QString nid2, QString type);
	bool areSiblings(QString nid1, QString nid2);

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

	// visualize
	QString toGraphvizFormat(QString subcaption, QString caption);
	void saveAsGraphviz(QString fname, QString subcaption = "", QString caption = "");
	void saveAsImage(QString fname);

public:
	static QString dotPath;
};

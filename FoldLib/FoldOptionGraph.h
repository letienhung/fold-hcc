#pragma once
#include "Graph.h"

class FoldEntity : public Structure::Node
{
public:
	FoldEntity(int cIdx, QString id);
	FoldEntity(FoldEntity &other);
	Node* clone();

	// entity index
	int entityIdx; 
};

class FoldOption : public Structure::Node
{
public:
	FoldOption(QString id);
	FoldOption(int hIdx, bool right, double s, double p, int n, QString id);
	FoldOption(FoldOption &other);
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
	void addNode(FoldEntity* cn);
	void addNode(FoldOption* fn);
	void addNode(BarrierNode* bn);
	void addFoldLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	// verifier
	bool verifyNodeType(QString nid, QString type);
	bool verifyLinkType(QString nid1, QString nid2, QString type);
	bool areSiblings(QString nid1, QString nid2);

	// getters
	FoldEntity*		getFoldEntiry(QString fnid);		// chain node of a folding node
	BarrierNode*	getBarrierNode(int fIdx);		// barrier node with face index
	QVector<FoldEntity*>		getAllFoldEntities();		
	QVector<FoldOption*>	getAllFoldOptions();	
	QVector<BarrierNode*>	getAllBarrierNodes();
	QVector<FoldOption*>	getSiblings(QString fnid);		// siblings of folding node
	QVector<FoldOption*>	getFoldingNodes(QString cnid);	// folding nodes of a chain node
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

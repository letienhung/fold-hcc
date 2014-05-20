#pragma once
#include "Graph.h"
#include "FdUtility.h"

class ChainNode : public Structure::Node
{
public:
	ChainNode(int idx, QString id);
	ChainNode(ChainNode &other);
	Node* clone();

	// chain index
	int chainIdx; 
};

class FoldOption : public Structure::Node
{
public:
	FoldOption(QString id);
	FoldOption(int hIdx, bool right, double s, double p, int n, QString id);
	FoldOption(FoldOption &other);
	Node* clone();

	double getCost();// cost

public:
	// idx of root segment
	int jointAxisIdx;

	// to which side to fold
	bool rightSide;

	// deformation
	double scale;
	double position;

	// #splits
	int nSplits;

	// duration
	TimeInterval duration;

	// fold region
	Geom::Rectangle region;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
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
	void addNode(FoldOption* fn);
	void addFoldLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	void clearCollisionLinks();

	// verifier
	bool verifyNodeType(QString nid, QString type);
	bool verifyLinkType(QString nid1, QString nid2, QString type);
	bool areSiblings(QString nid1, QString nid2);
	bool hasFreeFoldOptions(QString cnid);

	// getters
	ChainNode*		getChainNode(QString fnid);		// chain node of a folding node
	QVector<ChainNode*>	getAllChainNodes();		
	QVector<FoldOption*>	getAllFoldOptions();	
	QVector<FoldOption*>	getSiblings(QString fnid);		// siblings of folding node
	QVector<FoldOption*>	getFoldOptions(QString cnid);	// fold options of a chain node
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

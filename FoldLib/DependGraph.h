#pragma once
#include "Graph.h"
#include <QProcess>

enum FD_DIRECTION{FD_LEFT, FD_RIGHT};

class ChainNode : public Structure::Node
{
public:
	ChainNode(int idx, QString id);
	ChainNode(ChainNode &other);
	Node* clone();

	// chain index
	int chainIdx; 
};

class FoldingNode : public Structure::Node
{
public:
	FoldingNode(int hidx, FD_DIRECTION d, QString id);
	FoldingNode(FoldingNode &other);
	Node* clone();

	// hinge info
	int hingeIdx;
	FD_DIRECTION direct;

	// score
	double score;
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
	void addFoldingLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	// getters
	QVector<FoldingNode*>	getAllFoldingNodes();	// all folding nodes
	QVector<FoldingNode*>	getFoldingNodes(QString cnid); // folding nodes of a chain node
	QVector<ChainNode*>		getAllChainNodes();	// all chain nodes
	ChainNode*				getChainNode(QString fnid); // chain node of a folding node
	QVector<Structure::Link*> getFoldinglinks(QString nid);
	QVector<Structure::Link*> getCollisionLinks(QString nid);

	// gain of each folding node
	int computeGain(FoldingNode* fnode);
	void computeScores();
	bool isFreeChainNode(QString cnid);

	// visualize
	QString toGraphvizFormat(QString subcaption, QString caption);
	void saveAsGraphviz(QString fname, QString subcaption = "", QString caption = "");
	void saveAsImage(QString fname);

public:
	static QString dotPath;
};
#pragma once
#include "Graph.h"
#include <QProcess>

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
    DependGraph(QString id = "");
	DependGraph(DependGraph& other);
	~DependGraph();
	Graph* clone();

	void addNode(ChainNode* cn);
	void addNode(FoldingNode* fn);
	void addFoldingLink(Structure::Node* n1, Structure::Node* n2);
	void addCollisionLink(Structure::Node* n1, Structure::Node* n2);

	QVector<FoldingNode*>	getAllFoldingNodes();	// all folding nodes
	QVector<ChainNode*>		getAllChainNodes();	// all chain nodes
	QVector<FoldingNode*>	getFoldingNodes(QString cnid); // folding nodes of a chain node
	ChainNode*				getChainNode(QString fnid); // chain node of a folding node
	QVector<Structure::Link*> getFoldinglinks(QString nid);
	QVector<Structure::Link*> getCollisionLinks(QString nid);

	QString toGraphvizFormat(QString subcaption, QString caption);
	void saveAsGraphviz(QString fname, QString subcaption = "", QString caption = "");
	void saveAsImage(QString fname);

public:
	static QString dotPath;
};




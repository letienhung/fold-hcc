#pragma once

#include "FoldabilizerLibGlobal.h"

#include "Node.h"
#include "Link.h"

class Graph
{
public:
    Graph();
    Graph(QString fname);
    ~Graph();
	
public:
	// Modifier
    void addNode(Node* node);
    void addLink(Link* link);
	void addLink(QString nid1, QString nid2);
	void removeNode(QString nodeID);
	void removeLink(QString nid1, QString nid2);

	// Accessors
	Node* getNode(QString id);
	Link* getLink(QString nid1, QString nid2);
    QVector<Node *> getAdjacentNodes(QString nodeID);
    QVector<Node *> getLeafnode();
    QVector<Link*> getLinks(Node* n);

	// Parse from file
	bool parseHCC(QString fname);

	// Visualize
	void draw();
	void makeL();
	void makeT();
	void makeX();
	void makeChair();

	// Geometry property
	bool isDrawAABB;
	Point bbmin, bbmax, center;
	Scalar radius;
	void computeAABB();


public:
    QVector<Node*> nodes;
    QVector<Link*> links;

private:
	void clear();
};

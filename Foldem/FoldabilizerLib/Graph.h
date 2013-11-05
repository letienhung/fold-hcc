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
	void addLink(QString nid1, QString nid2);
	void removeNode(QString nodeID);
	void removeLink(Link* link);

	// Accessor
	Node* getNode(QString id);
	Link* getLink(QString nid1, QString nid2);

	// Parse from file
	bool parseHCC(QString fname);

	// Visualize
	void draw();
	void makeL();
	void makeT();
	void makeX();
	void makeU();
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

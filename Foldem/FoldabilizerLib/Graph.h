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
	void removeNode(QString nodeID);
	void removeLink(Link* link);

	// Accessors
	Node* getNode(QString id);
	Link* getLink(QString nid1, QString nid2);
	QVector<Link*> getLinks(QString nodeID);

	// Parse from file
	bool parseHCC(QString fname);
	// Save HCC file
	bool saveHCC(QString fname);

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

	// Jump
	void jump();
	void restoreConfiguration();
	bool isEmpty();
	void resetTags();

public:
    QVector<Node*> nodes;
    QVector<Link*> links;

private:
	void clear();

};

#pragma once

#include "FoldabilizerLibGlobal.h"

#include "Node.h"
#include "Link.h"

class Graph
{
public:
    Graph();
    Graph(QString &fname);
    ~Graph();
	
public:
	// Parse from file
	bool parseHCC(QString &fname);

	// Add elements
	void addNode(QString& id, Box& b);
    void addNode(Node* node);
	void removeNode(Node* node);
    void addLink(int id, Link* l);
	//Get node with id
	Node* getNode(QString& id);

	//Get the list of adjacent nodes of the given node
    QVector<Node *> getAdjnode(Node* node);
	//Get the list of leaf CCs within the HCC graph
    QVector<Node *> getLeafnode();
    //Get the links of a node
    QVector<Link*> getLinks(Node* n);

	// Visualize
	void draw();
	void makeL();

public:
    QVector<Node*> nodes;
    QVector<Link*> links;

private:
	void clearList();

};

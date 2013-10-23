#pragma once

#include "FoldabilizerLibGlobal.h"

#include "Node.h"
#include "Edge.h"

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
	void addNode(QString id, Box b);
    void addNode(Node* node);
	void removeNode(Node* node);
	void addEdge();

	//Get the list of adjacent nodes of the given node
    std::vector<Node *> getAdjnode(Node* node);
	//Get the list of leaf CCs within the HCC graph
    std::vector<Node *> getLeafnode();

	QVector<Edge*> getEdges(Node* n);

public:
    QVector<Node*> nodes;
	QVector<Edge*> edges;

private:
	void clearList();
};

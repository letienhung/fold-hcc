#pragma once

#include "Node.h"

class Graph
{
public:
    Graph();
    Graph(QString &fname);
    ~Graph();
	
public:
	bool parseHCC(QString &fname);
    void addNode(Node* node);
	//void updateHCC(CuboidNode* mCC);

	//Get the list of adjacent nodes of the given node
    std::vector<Node *> getAdjnode(Node* node);
	//Get the list of leaf CCs within the HCC graph
    std::vector<Node *> getLeafnode();

public:
    std::vector<Node*> nodeList;

private:
	void clearList();
};

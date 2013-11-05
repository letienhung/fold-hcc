#pragma once

#include "FoldabilizerLibGlobal.h"

class Node;

class Link
{
public:
    Link();
    Link(Node* n1, Node* n2);
    ~Link(){}

public:
	bool  hasNode(QString nodeID);
	Node* getNode(QString nodeID);
	Node* otherNode(QString nodeID);

	void draw();
		
public:
    Node *node1;
    Node *node2;

	Point orig;
	Vector3 
};


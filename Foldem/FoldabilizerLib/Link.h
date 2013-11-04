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
	double angle;

public:
    void setNode(Node* n1, Node* n2);

    Node* getNode1();
    Node* getNode2();

	void draw();
						  
private:
    Node *node1;
    Node *node2;
};


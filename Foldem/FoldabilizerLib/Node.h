#pragma once

#include "Box.h"
#include "FoldabilizerLibGlobal.h"

class Edge;

class Node
{
public:
	Node(){}
    Node(Box &b, QVector<Edge* > &eList);
    ~Node();

public:
    //Get the list of adjacent nodes
    QVector<Node *> getAdjnodes();

private:
    Box mBox; 

public:
    QVector<Edge* > edgeList;

};


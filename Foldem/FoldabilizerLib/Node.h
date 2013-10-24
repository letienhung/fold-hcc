#pragma once

#include "Box.h"
#include "FoldabilizerLibGlobal.h"

#include "Link.h"

//class Link;

class Node
{
public:
	Node(){}
    Node(Box &b, QVector<Link* > &lList);
    ~Node();

public:
    //Get the list of adjacent nodes
    QVector<Node *> getAdjnodes();

private:
    Box mBox; 

public:
    QVector<Link* > linkList;
	QString id;

};


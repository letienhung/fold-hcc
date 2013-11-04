#pragma once

#include "Box.h"
#include "FoldabilizerLibGlobal.h"

#include "Link.h"

//class Link;

class Node
{
public:
	Node(){}
    Node(Box b, QString id);
    ~Node();

public:
    //Get the list of adjacent nodes
    QVector<Node *> getAdjnodes();

	// Visualize
	void draw();
	void drawBox();

	// Geoemtry
	QVector<Point> getBoxConners();
	QVector< QVector<Point> > getBoxFaces();

private:
    Box mBox; 

public:
    QVector<Link* > linkList;
	QString mID;

};

// Vertex ID of face corners
static uint cubeIds[6][4] = 
{
	1, 2, 6, 5,
	0, 4, 7, 3,
	4, 5, 6, 7,
	0, 3, 2, 1,
	0, 1, 5, 4,
	2, 3, 7, 6
};
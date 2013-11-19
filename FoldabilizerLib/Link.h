#pragma once

#include "FoldabilizerLibGlobal.h"
#include "Frame.h"
#include "Box.h"
#include "Hinge.h"

class Node;

class Link
{
public:
	QString	id;
	Node	*node1, *node2;

	bool isFixed;				// tags used for restore configuration
	bool isBroken;
	bool isNailed;

    Link(Node* n1, Node* n2, Point c, Vec3d a);
	Link(Node* n1, Node* n2);
    ~Link(){};

	// accessors
	bool  hasNode(QString nodeID);
	Node* getNode(QString nodeID);
	Node* otherNode(QString nodeID);

	// hinges
	QVector<Hinge>	hinges;
	Hinge activeHinge();

	// fix
	bool fix();

	// visualization
	void draw(double scale);
};


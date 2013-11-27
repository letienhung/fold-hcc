#pragma once

#include "UtilityGlobal.h"
#include "Frame.h"
#include "Box.h"
#include "Hinge.h"

class Node;

class Link
{
public:
	QString	id;
	Node	*node1, *node2;
	QVector<Hinge*>	hinges;
	int activeHingeID;

	Link(Node* n1, Node* n2);
	~Link();

	// accessors
	bool  hasNode(QString nodeID);
	Node* getNode(QString nodeID);
	Node* otherNode(QString nodeID);

	// hinges
	int		nbHinges();
	int		getActiveHingeId();
	void	setActiveHingeId(int hid);
	Hinge*	activeHinge();
	void	detectHinges(bool ee = true, bool ef = true, bool ff = true);

	// restore configuration
	bool isFixed;				
	bool isBroken;
	bool isNailed;
	bool fix();

	// hot link: incident to hot nodes
	bool isHot;

	// visualization
	void setHingeScale(double scale);
	void draw();
};


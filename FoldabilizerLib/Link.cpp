#include "Link.h"
#include "Node.h"
#include "../CustomDrawObjects.h"
#include "HingeDetector.h"

Link::Link(Node* n1, Node* n2, Point c, Vec3d a)
{
	// depreciated 
}

Link::Link( Node* n1, Node* n2 )
{
	this->node1 = n1;
	this->node2 = n2;
	this->id = node1->mID + ":" + node2->mID;

	hinges = HingeDetector::getHinges(node1, node2);

	// tag
	isFixed = false;
	isBroken = false;
	isNailed = false;
}

void Link::draw(double scale)
{
	if (isBroken || isNailed) return;
	foreach(Hinge h, hinges) h.draw(scale, false);
}

bool Link::hasNode( QString nodeID )
{
	return node1->mID == nodeID || node2->mID == nodeID;
}

Node* Link::otherNode( QString nodeID )
{
	return (nodeID == node1->mID)? node2 : node1;
}

Node* Link::getNode( QString nodeID )
{
	return (nodeID == node1->mID)? node1 : node2;
}

bool Link::fix()
{
	return true;
}

Hinge Link::activeHinge()
{
	return hinges[0];
}

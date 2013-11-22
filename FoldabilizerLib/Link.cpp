#include "Link.h"
#include "Node.h"
#include "CustomDrawObjects.h"
#include "HingeDetector.h"

using namespace Geom;

Link::Link( Node* n1, Node* n2 )
{
	this->node1 = n1;
	this->node2 = n2;
	this->id = node1->mID + ":" + node2->mID;

	HingeDetector hinge_detector(node1, node2);
	hinges = hinge_detector.getHinges();
	activeHingeID = 0;

	// tag
	isFixed = false;
	isBroken = false;
	isNailed = false;
	isHot = false;
}

void Link::draw()
{
	if (isBroken || isNailed) return;
	foreach(Hinge* h, hinges) h->draw(false);

	if (this->activeHinge())
		this->activeHinge()->draw(true);
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
	return this->activeHinge()->fix();
}

int Link::nbHinges()
{
	return hinges.size();
}

Link::~Link()
{
	foreach(Hinge* h, hinges) 
		delete h;
}

void Link::setActiveHingeId( int hid )
{
	if (hid >= 0 && hid < this->nbHinges())
		this->activeHingeID = hid;
}

Hinge* Link::activeHinge()
{
	if (this->hinges.isEmpty())
		return NULL;
	else
		return this->hinges[activeHingeID];
}

int Link::getActiveHingeId()
{
	return activeHingeID;
}

void Link::setHingeScale( double scale )
{
	foreach(Hinge* h, hinges)
		h->scale = scale;
}

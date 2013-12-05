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

	this->detectHinges();
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
	foreach(Hinge* h, hinges) h->draw();

	if (this->activeHinge())
		this->activeHinge()->draw();
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

void Link::detectHinges( bool ee, bool ef, bool ff )
{
	foreach (Hinge* h, this->hinges) delete h;
	this->hinges.clear();

	HingeDetector hinge_detector(node1, node2);
	hinges = hinge_detector.getHinges(ee, ef, ff);
	this->filterHinges();
}

void Link::filterHinges()
{
	//qDebug() << "Filter hinges between " << this->id;
	//int origN = hinges.size();

	double maxExt = std::numeric_limits<double>::min();
	foreach(Hinge* h, hinges){
		if (h->extent > maxExt)	maxExt = h->extent;		
	}

	QVector<Hinge*> fH;
	double threshold = maxExt / 10;
	foreach(Hinge* h, hinges){
		if (h->extent > threshold)
			fH.push_back(h);
		else
			delete h;
	}
	this->hinges = fH;

	//qDebug() << "\t" << origN << " => " << hinges.size();
}

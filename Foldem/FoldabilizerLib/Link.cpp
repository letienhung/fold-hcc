#include "Link.h"
#include "Node.h"
#include "../CustomDrawObjects.h"

Link::Link(Node* n1, Node* n2, Point c, Vec3d a)
	:node1(n1), node2(n2), center(c), axis(a)
{
	// dihedral directions
	this->v1 = node1->dihedralDirection(this->axis).normalized();
	this->v2 = node2->dihedralDirection(this->axis).normalized();

	// dihedral frames
}

void Link::draw()
{
	FrameSoup fs(1);
	fs.addFrame(axis, v1, v2, center);
	fs.draw();
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

#include "Link.h"
#include "Node.h"

Link::Link()
{
    node1 = NULL;
    node2 = NULL;
}

Link::Link(Node* n1, Node* n2)
{
    node1 = n1;
    node2 = n2;
}

void Link::draw()
{
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

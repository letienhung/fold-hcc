#include "Link.h"
#include "Node.h"

Link::Link( Node* n1, Node* n2 )
{
	this->node1 = n1;
	this->node2 = n2;
	this->id = node1->id + ":" + node2->id;
}


bool Link::hasNode( QString nid )
{
	return node1->hasId(nid) || node2->hasId(nid);
}

Node* Link::getNode( QString nid )
{
	return (node1->hasId(nid))? node1 : node2;
}

#include "Link.h"
#include "Node.h"

Structure::Link::Link( Structure::Node* n1, Structure::Node* n2 )
{
	this->node1 = n1;
	this->node2 = n2;
	this->id = node1->id + ":" + node2->id;
}


bool Structure::Link::hasNode( QString nid )
{
	return node1->hasId(nid) || node2->hasId(nid);
}

Structure::Node* Structure::Link::getNode( QString nid )
{
	return (node1->hasId(nid))? node1 : node2;
}

Structure::Node* Structure::Link::getNodeOther( QString nid )
{
	return (node1->hasId(nid))? node2 : node1;
}

#include "Link.h"
#include "Node.h"

Structure::Link::Link( Structure::Node* n1, Structure::Node* n2 )
{
	nid1 = n1->mID;
	nid2 = n2->mID;
	node1 = n1;
	node2 = n2;

	id = node1->mID + ":" + node2->mID;
}

Structure::Link::Link( Link& other )
{
	// note: pointers to nodes are not copied
	nid1 = other.nid1;
	nid2 = other.nid2;
	id = other.id;
}

Structure::Link* Structure::Link::clone()
{
	return new Link(*this);
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

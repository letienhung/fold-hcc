#pragma once

#include "Node.h"

namespace Structure{

class Link : public PropertyMap
{
public:
	Link(Node* n1, Node* n2);
	virtual ~Link();
	Link(Link& other);
	virtual Link* clone();

	// accessors
	bool  hasNode(QString nid);
	Node* getNode(QString nid);
	Node* getNodeOther(QString nid);

	// visualize
	virtual void draw(){}

public:
	QString	id;
	QString nid1, nid2;
	Node *node1, *node2;
};

}

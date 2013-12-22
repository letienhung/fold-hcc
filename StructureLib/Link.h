#pragma once

#include "Node.h"

namespace Structure{

class Link
{
public:
	Link(Node* n1, Node* n2);
	Link(Link& other);
	virtual Link* clone();

	bool  hasNode(QString nid);
	Node* getNode(QString nid);
	Node* getNodeOther(QString nid);

	virtual void draw(){}

public:
	QString	id;
	QString nid1, nid2;
	Node *node1, *node2;

    PropertyMap properties;
};

}

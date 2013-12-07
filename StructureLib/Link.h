#pragma once

#include <QString>

namespace Structure{

class Node;
class Link
{
public:
	Link(Node* n1, Node* n2);

	bool  hasNode(QString nid);
	Node* getNode(QString nid);
	Node* getNodeOther(QString nid);

	virtual void draw(){}

public:
	QString	id;
	Node	*node1, *node2;
};

}
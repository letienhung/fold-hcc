#pragma once

#include <QVector>

#include "Node.h"
#include "Link.h"

namespace Structure{

class Graph
{
public:
    Graph();

public:
	// Modifier
	void addNode(Node* node);
	void addLink(Link* link);
	void addLink(Node* n0, Node* n1);
	void removeNode(QString nid);
	void removeLink(Link* link);
	void clear();

	// Accessors
	int		nbNodes();
	int		nbLinks();
	bool	isEmpty();
	Node*	getNode(int idx);
	Node*	getNode(QString nid);
	Link*	getLink(QString nid1, QString nid2);
	QVector<Link*> getLinks(QString nid);

	// Visualize
	bool isDraw;
	void draw();

protected:
	QVector<Node*> nodes;
	QVector<Link*> links;
};

}
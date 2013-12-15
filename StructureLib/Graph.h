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
	void replaceNode(Node* old_node, Node* new_node);
	void removeLink(Link* link);
	void clear();

	// Accessors
	int		nbNodes();
	int		nbLinks();
	bool	isEmpty();
	int		getNodeIndex(Node* node);
	int		getLinkIndex(Link* link);
	Node*	getNode(int idx);
	Node*	getNode(QString nid);
	Link*	getLink(QString nid1, QString nid2);
	QVector<Link*> getLinks(QString nid);
	QVector<Node*> getNeighbourNodes(Node* node);

	// Visualize
	bool isDraw;
	virtual void draw();

	// Selection
	void drawWithNames();
	void selectNode(int nid);
	QVector<Node*> selectedNodes();

public:
	QVector<Node*> nodes;
	QVector<Link*> links;
};

}
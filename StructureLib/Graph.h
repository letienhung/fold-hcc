#pragma once

#include <QVector>

#include "Node.h"
#include "Link.h"

namespace Structure{

class Graph : public PropertyMap
{
public:
    Graph(QString id = "");
	Graph(Graph& other);
	virtual Graph* clone();

public:
	// Modifier
	void addNode(Node* node);
	void addLink(Link* link);
	Link* addLink(Node* n1, Node* n2);
	Link* addLink(QString nid1, QString nid2);
	void removeNode(QString nid);
	void replaceNode(Node* old_node, Node* new_node);
	bool removeLink(Link* link);
	bool removeLink(QString nid1, QString nid2);
	void clear();
	void clearLinks();

	// Accessors
	int		nbNodes();
	int		nbLinks();
	bool	isEmpty();
	bool	containsNode(QString nid);
	int		getNodeIndex(QString nid);
	int		getLinkIndex(QString nid1, QString nid2);
	Node*	getNode(int idx);
	Node*	getNode(QString nid);
	Link*	getLink(QString nid1, QString nid2);
	QVector<Link*> getLinks(QString nid);
	QVector<Node*> getNeighbourNodes(Node* node);
	QVector<Node*> getConnectedNodes(Node* seed);
	QVector< QVector<Node*> > getConnectedComponents();
	QVector<Node*> getNodesWithTag(QString tag);

	// Visualize
	bool isDraw;
	virtual void draw();

	// Selection
	void drawWithNames();
	bool selectNode(int nid);
	void deselectAllNodes();
	QVector<Node*> getSelectedNodes();

	// cycles
	QVector<QVector<QString> > findCycleBase();

public:
	QString mID;
	QVector<Node*> nodes;
	QVector<Link*> links;
};

}
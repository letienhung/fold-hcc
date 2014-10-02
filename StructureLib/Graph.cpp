#include "Graph.h"
#include <QtOpenGL/qgl.h>
#include <QQueue>
#include <QStack>
#include <QSet>

Structure::Graph::Graph(QString id)
{
	this->mID = id;
}


Structure::Graph::~Graph()
{
	foreach (Link* l, links) delete l;
	foreach (Node* n, nodes) delete n;
}


Structure::Graph::Graph(Graph& other)
{
	this->mID = other.mID;

	foreach (Node* n, other.nodes)
		addNode(n->clone());

	foreach (Link* l, other.links)	
	{
		Link* l_copy = l->clone();
		addLink(l_copy);
		l_copy->node1 = getNode(l->nid1);
		l_copy->node2 = getNode(l->nid2);
	}

	properties = other.properties; 
}

Structure::Graph* Structure::Graph::clone()
{
	return new Graph(*this);
}


void Structure::Graph::clearLinks()
{
	links.clear();
}

void Structure::Graph::clear()
{
	nodes.clear();
	clearLinks();
}

void Structure::Graph::addNode(Node* node)
{
	nodes.push_back(node);
}

void Structure::Graph::addLink( Link* link)
{
	links.push_back(link);
}

Structure::Link* Structure::Graph::addLink( Node* n1, Node* n2 )
{
	Link* link = new Link(n1, n2);
	links.push_back(link);

	return link;
}

Structure::Link* Structure::Graph::addLink( QString nid1, QString nid2 )
{
	Structure::Node* n1 = getNode(nid1);
	Structure::Node* n2 = getNode(nid2);
	if (n1 && n2)
		return addLink(n1, n2);
	else
		return nullptr;
}

int Structure::Graph::getNodeIndex( QString nid )
{
	int idx = -1;
	for (int i = 0; i < nodes.size(); i++)	{
		if (nodes[i]->mID == nid) {
			idx = i; 
			break;
		}
	}

	return idx;
}


bool Structure::Graph::containsNode( QString nid )
{
	int i = getNodeIndex(nid);
	return (i >= 0) && (i < nodes.size());
}


int Structure::Graph::getLinkIndex( QString nid1, QString nid2 )
{
	int idx = -1;
	for (int i = 0; i < links.size(); i++)	{
		if (links[i]->hasNode(nid1) && links[i]->hasNode(nid2)) {
			idx = i;
			break;
		}
	}

	return idx;
}


void Structure::Graph::removeNode( QString nid )
{
	// node index
	int idx = getNodeIndex(nid);
	if (idx == -1) return;

	// remove incident links
	foreach(Link* l, links)	{
		if (l->hasNode(nid)) removeLink(l);
	}

	// remove
	//delete nodes[idx];
	nodes.remove(idx);
}

bool Structure::Graph::removeLink( Link* link )
{
	if (link == nullptr) return false;

	// link index
	int idx = getLinkIndex(link->nid1, link->nid2);
	if (idx == -1) return false;

	// deallocate
	// delete links[idx];
	links.remove(idx);
	return true;
}

bool Structure::Graph::removeLink( QString nid1, QString nid2 )
{
	// link index
	int idx = getLinkIndex(nid1, nid2);
	if (idx == -1) return false;

	// deallocate
	// delete links[idx];
	links.remove(idx);
	return true;
}

Structure::Node* Structure::Graph::getNode(QString nid)
{
	foreach(Node* n, nodes)
		if(n->hasId(nid)) return n;

	return nullptr;
}

Structure::Node* Structure::Graph::getNode( int idx )
{
	if (idx >= 0 && idx < nbNodes())
		return this->nodes[idx];
	else
		return nullptr;
}

Structure::Link* Structure::Graph::getLink( QString nid1, QString nid2 )
{
	foreach(Link* l, links)	{
		if (l->hasNode(nid1) && l->hasNode(nid2))
			return l;
	}

	return nullptr;
}

QVector<Structure::Link*> Structure::Graph::getLinks( QString nid )
{
	QVector<Link*> ls;
	foreach(Link* l, links){
		if (l->hasNode(nid)) ls.push_back(l);
	}

	return ls;
}

int Structure::Graph::nbNodes()
{
	return nodes.size();
}

int Structure::Graph::nbLinks()
{
	return links.size();
}

bool Structure::Graph::isEmpty()
{
	return nodes.isEmpty();
}

void Structure::Graph::draw()
{
	foreach(Link *l, links) 
		l->draw();
	foreach(Node *n, nodes)
		n->draw();
}

void Structure::Graph::drawWithNames()
{
	for (int i = 0; i < nbNodes(); i++)
	{
		nodes[i]->drawWithName(i);
	}
}

bool Structure::Graph::selectNode( int nid )
{
	if (nid >= 0 && nid < nbNodes())
	{
		nodes[nid]->flipSelect();
		return nodes[nid]->isSelected;
	}

	return false;
}


void Structure::Graph::deselectAllNodes()
{
	foreach (Node* n, nodes) n->isSelected = false;
}


QVector<Structure::Node*> Structure::Graph::getSelectedNodes()
{
	QVector<Node*> sn;
	foreach(Node* n, nodes)
	{
		if (n->isSelected)
			sn.push_back(n);
	}

	return sn;
}

void Structure::Graph::replaceNode( Node* old_node, Node* new_node )
{
	QVector<Node*> neighbours = getNeighbourNodes(old_node);

	removeNode(old_node->mID);
	addNode(new_node);
	
	foreach(Node* n, neighbours)
		addLink(n, new_node);
}

QVector<Structure::Node*> Structure::Graph::getNeighbourNodes( Node* node )
{
	QVector<Node*> neighbours;
	foreach (Link* l, getLinks(node->mID))
		neighbours.push_back(l->getNodeOther(node->mID));

	return neighbours;
}

QVector< QVector<Structure::Node*> > Structure::Graph::getComponents()
{
	QVector< QVector<Structure::Node*> > cs;

	// set tag
	QString visitedTag = "visitedtagforgraphcomponent";
	foreach (Node* n, nodes) n->removeTag(visitedTag);

	// find all components
	foreach(Node* n, nodes)
	{
		if (n->hasTag(visitedTag)) continue;

		// nodes connecting to n
		QVector<Node*> component;
		QQueue<Node*> activeNodes;
		activeNodes.enqueue(n);
		while (!activeNodes.isEmpty())
		{
			Node* curr = activeNodes.dequeue();
			component << curr;
			curr->addTag(visitedTag);

			foreach (Node* nei, getNeighbourNodes(curr))
			{
				if (!nei->hasTag(visitedTag))
				{
					activeNodes.enqueue(nei);
					nei->addTag(visitedTag);
				}
			}
		}

		// store
		cs << component;
	}

	// clean up
	foreach (Node* n, nodes) n->removeTag(visitedTag);

	return cs;
}

QVector<Structure::Node*> Structure::Graph::getNodesWithTag( QString tag )
{
	QVector<Node*> ns;
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->hasTag(tag)) ns << nodes[i];
	}
	return ns;
}

QVector<QVector<QString> > Structure::Graph::findCycleBase()
{
	QMap<QString, QSet<QString>> used;
	QMap<QString, QString> parent;
	QStack<QString> stack;
	QVector<QVector<QString>> cycles;

	foreach (Node* root_node, nodes) {
		QString root = root_node->mID;
		// Loop over the connected
		// components of the graph.
		if (parent.contains(root)) {
			continue;
		}
		// Free some memory in case of
		// multiple connected components.
		used.clear();
		// Prepare to walk the spanning tree.
		parent[root] = root;
		used[root] = QSet<QString>();
		stack.push(root);
		// Do the walk. It is a BFS with
		// a LIFO instead of the usual
		// FIFO. Thus it is easier to 
		// find the cycles in the tree.
		while (!stack.isEmpty()) {
			QString current = stack.pop();
			QSet<QString> currentUsed = used[current];
			foreach (Link* e, getLinks(current)) {
				QString neighbor = e->getNodeOther(current)->mID;
				if (!used.contains(neighbor)) {
					// found a new node
					parent[neighbor] = current;
					QSet<QString> neighbourUsed;
					neighbourUsed << current;
					used[neighbor] = neighbourUsed;
					stack.push(neighbor);
				}
				else if (neighbor == current) {
					// found a self loop
					QVector<QString> cycle;
					cycle << current;
					cycles << cycle;
				}
				else if (!currentUsed.contains(neighbor)) {
					// found a cycle
					QSet<QString>& neighbourUsed = used[neighbor];
					QVector<QString> cycle;
					cycle << neighbor;
					cycle << current;
					QString p = parent[current];
					while (!neighbourUsed.contains(p)) {
						cycle << p;
						p = parent[p];
					}
					cycle << p;
					cycles << cycle;
					neighbourUsed << current;
				}
			}
		}
	}
	return cycles;
}

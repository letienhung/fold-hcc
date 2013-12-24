#include "Graph.h"
#include <QtOpenGL/qgl.h>

Structure::Graph::Graph(QString id)
{
	this->mID = id;
}

Structure::Graph::Graph(Graph& other)
{
	this->mID = other.mID;

	foreach (Node* n, other.nodes)
		addNode(n->clone());

	foreach (Link* l, other.links)	
	{
		addLink(l->clone());
		l->node1 = getNode(l->nid1);
		l->node2 = getNode(l->nid2);
	}
}

Structure::Graph* Structure::Graph::clone()
{
	return new Graph(*this);
}


void Structure::Graph::clear()
{
	nodes.clear();
	links.clear();
}

void Structure::Graph::addNode(Node* node)
{
	nodes.push_back(node);
}

void Structure::Graph::addLink( Link* link)
{
	links.push_back(link);
}

void Structure::Graph::addLink( Node* n1, Node* n2 )
{
	links.push_back(new Link(n1, n2));
}

int Structure::Graph::getNodeIndex( Node* node )
{
	int idx = -1;
	for (int i = 0; i < nodes.size(); i++)	{
		if (nodes[i] == node) {
			idx = i;
			break;
		}
	}

	return idx;
}

int Structure::Graph::getLinkIndex( Link* link )
{
	int idx = -1;
	for (int i = 0; i < links.size(); i++)	{
		if (links[i] == link) {
			idx = i;
			break;
		}
	}

	return idx;
}


void Structure::Graph::removeNode( QString nid )
{
	// node index
	int idx = getNodeIndex(getNode(nid));
	if (idx == -1) return;

	// remove incident links
	foreach(Link* l, links)	{
		if (l->hasNode(nid)) removeLink(l);
	}

	// remove
	//delete nodes[idx];
	nodes.remove(idx);
}

void Structure::Graph::removeLink( Link* link )
{
	// link index
	int idx = getLinkIndex(link);
	if (idx == -1) return;

	// deallocate
	// delete links[idx];
	links.remove(idx);
}

Structure::Node* Structure::Graph::getNode(QString nid)
{
	foreach(Node* n, nodes)
		if(n->hasId(nid)) return n;

	return NULL;
}

Structure::Node* Structure::Graph::getNode( int idx )
{
	if (idx >= 0 && idx < nbNodes())
		return this->nodes[idx];
	else
		return NULL;
}

Structure::Link* Structure::Graph::getLink( QString nid1, QString nid2 )
{
	foreach(Link* l, links)	{
		if (l->hasNode(nid1) && l->hasNode(nid2))
			return l;
	}

	return NULL;
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

void Structure::Graph::selectNode( int nid )
{
	if (nid >= 0 && nid < nbNodes())
	{
		nodes[nid]->flipSelect();
	}
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

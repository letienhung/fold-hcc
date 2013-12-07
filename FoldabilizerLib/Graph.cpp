#include "Graph.h"

Graph::Graph()
{
}



void Graph::clear()
{
	foreach(Node* n, nodes) delete n;
	foreach(Link* l, links) delete l;

	nodes.clear();
	links.clear();
}

void Graph::addNode(Node* node)
{
	nodes.push_back(node);
}

void Graph::addLink( Link* link)
{
	links.push_back(link);
}

void Graph::addLink( Node* n0, Node* n1 )
{
	links.push_back(new Link(n0, n1));
}

void Graph::removeNode( QString nodeID )
{
	// remove incident links
	foreach(Link* l, links)	{
		if (l->hasNode(nodeID))
			removeLink(l);
	}

	// node index
	int idx = -1;
	for (int i = 0; i < nodes.size(); i++)	{
		if (nodes[i]->hasId(nodeID)) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;

	// deallocate and remove
	delete nodes[idx];
	nodes.remove(idx);
}

void Graph::removeLink( Link* link )
{
	// find and remove
	for (int i = 0; i < links.size(); i++)	{
		if (links[i] == link) {
			links.remove(i);
			break;
		}
	}

	// deallocate
	delete link;
}

Node* Graph::getNode(QString nid)
{
	foreach(Node* n, nodes)
		if(n->hasId(nid)) return n;

	return NULL;
}

Node* Graph::getNode( int idx )
{
	if (idx >= 0 && idx < nbNodes())
		return this->nodes[idx];
	else
		return NULL;
}

Link* Graph::getLink( QString nid1, QString nid2 )
{
	foreach(Link* l, links)	{
		if (l->hasNode(nid1) && l->hasNode(nid2))
			return l;
	}

	return NULL;
}

QVector<Link*> Graph::getLinks( QString nid )
{
	QVector<Link*> ls;
	foreach(Link* l, links){
		if (l->hasNode(nid)) ls.push_back(l);
	}

	return ls;
}

int Graph::nbNodes()
{
	return nodes.size();
}

int Graph::nbLinks()
{
	return links.size();
}

bool Graph::isEmpty()
{
	return nodes.isEmpty();
}

void Graph::draw()
{
	foreach(Link *l, links) l->draw();
	foreach(Node *n, nodes) n->draw();
}
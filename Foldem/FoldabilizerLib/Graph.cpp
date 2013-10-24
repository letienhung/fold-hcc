#include "Graph.h"

Graph::Graph()
{
	//Todo
}

Graph::Graph(QString &fname)
{
	if(!parseHCC(fname))
		qDebug()<<"Failed to load HCC file!\n";
}

Graph::~Graph()
{
    clearList();
}

void Graph::clearList()
{
    int nSize = nodes.size();
	if(nSize){
		for (int i = 0; i < nSize; i++)
			delete nodes[i];
		nodes.clear();
        links.clear();
	}
}

void Graph::addNode(Node* node)
{
    nodes.push_back(node);
}

bool Graph::parseHCC(QString &fname)
{
	//TODO
	return false;
}

QVector<Node *> Graph::getAdjnode(Node* node)
{
    return node->getAdjnodes();
}

QVector<Node *> Graph::getLeafnode()
{
	QVector<Node *> leafnodes;
	
    int nSize = nodes.size();
	for(int i = 0; i < nSize; i++)
        if (nodes[i]->linkList.size() == 1)
            leafnodes.push_back(nodes[i]);

   return leafnodes;
}

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
		edges.clear();
	}
}

void Graph::addCNode(Node* node)
{
    nodes.push_back(node);
}

bool Graph::parseHCC(QString &fname)
{
	//TODO
	return false;
}

std::vector<Node *> Graph::getAdjnode(Node* node);
{
    return node->getAdjnodes();
}

std::vector<Node *> Graph::getLeafnode()
{
	std::vector<Node *> leafnodes;
	
    int nSize = nodes.size();
	for(int i = 0; i < nSize; i++)
        if (nodes[i]->edgeList.size() == 1)
            leafnodes.push_back(nodes[i]);

   return leafnodes;
}

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
    if(nodeList.size())
		clearList();
}

void Graph::clearList()
{
    int mSize = nodeList.size();
	for (int i = 0; i < mSize; i++)
	{
        delete nodeList[i];
	}
    nodeList.clear();
}

void Graph::addCNode(Node* node)
{
    nodeList.push_back(node);
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
	std::vector<CuboidNode *> leafnodes;
	
    int mSize = nodeList.size();
	for(int i = 0; i < mSize; i++)
        if (nodeList[i]->HingeList.size() == 1)
            leafnodes.push_back(nodeList[i]);

   return leafnodes;
}

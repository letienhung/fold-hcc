#include "DependGraph.h"

DependGraph::DependGraph( LayerGraph* ly )
	:Graph(ly->mID)
{
}

DependGraph::DependGraph( DependGraph& other )
	:Graph(other)
{
	layer = other.layer;
}

Structure::Graph* DependGraph::clone()
{
	return new DependGraph(*this);
}

void DependGraph::addFoldingLink( Structure::Node* n1, Structure::Node* n2 )
{
	Structure::Link* link =  Graph::addLink(n1, n2);
	link->properties["type"] = "folding";
}

void DependGraph::addCollisionLink( Structure::Node* n1, Structure::Node* n2 )
{
	Structure::Link* link =  Graph::addLink(n1, n2);
	link->properties["type"] = "collision";
}

ChainNode* DependGraph::getChainNode( QString id )
{
	return (ChainNode*)getNode(id);
}

QVector<FoldingNode*> DependGraph::getFoldingNodes( QString id )
{
	//QVector<Structure::Node*> getNeighbourNodes(getNode(id));
}

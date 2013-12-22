#include "DepGraph.h"

#include <QStringList>

DepGraph::DepGraph()
{
}

DepGraph::DepGraph( DepGraph& other )
	:Graph(other)
{

}

Structure::Graph* DepGraph::clone()
{
	return new DepGraph(*this);
}

void DepGraph::addPartAndFolingNodes( QString pid, QStringList fids )
{

}

void DepGraph::addHingeLink( QString nid1, QString nid2 )
{
	 
}

void DepGraph::addCollisionLink( QString nid1, QString nid2 )
{

}

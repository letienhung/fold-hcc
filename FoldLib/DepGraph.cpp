#include "DepGraph.h"

#include <QStringList>

PizzaDyGraph::PizzaDyGraph()
{
}

PizzaDyGraph::PizzaDyGraph( PizzaDyGraph& other )
	:Graph(other)
{

}

Structure::Graph* PizzaDyGraph::clone()
{
	return new PizzaDyGraph(*this);
}


PizzaFNode::PizzaFNode( int hid, FD_DIRECTION d, QString id )
	:Structure::Node(id)
{
	hingeId = hid;
	direct = d;	
}
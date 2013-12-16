#include "Node.h"


Structure::Node::Node( QString nid )
{
	id = nid;
	isSelected = false;
}

Structure::Node::Node(Node &other)
{
	id = other.id;
	isSelected = false;
}

bool Structure::Node::hasId( QString id )
{ 
	return this->id == id;
}

void Structure::Node::flipSelect()
{
	isSelected = !isSelected;
}

void Structure::Node::draw()
{

}

void Structure::Node::drawWithName( int name )
{
	Q_UNUSED(name);
}

Structure::Node* Structure::Node::clone()
{
	return new Node(*this);
}

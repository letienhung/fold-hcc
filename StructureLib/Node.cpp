#include "Node.h"


Structure::Node::Node( QString nid )
{
	mID = nid;
	isSelected = false;
}

Structure::Node::Node(Node &other)
{
	mID = other.mID;
	isSelected = false;
}

bool Structure::Node::hasId( QString id )
{ 
	return this->mID == id;
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

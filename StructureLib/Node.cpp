#include "Node.h"


Structure::Node::Node( QString nid )
{
	mID = nid;
	isSelected = false;

	properties["isSplittable"] = true;
	properties["isScalable"] = true;
}

Structure::Node::Node(Node &other)
{
	mID = other.mID;
	isSelected = other.isSelected;
	properties = other.properties;

	properties["isSplittable"] = true;
	properties["isScalable"] = true;
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

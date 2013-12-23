#include "PizzaChain.h"

PizzaChain::PizzaChain( FdNode* part, FdNode* panel, QString id )
	:FdGraph(id)
{
	mPart = (FdNode*)part->clone();
	mPanel = (FdNode*)panel->clone();

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel);
}

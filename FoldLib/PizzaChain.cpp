#include "PizzaChain.h"
#include "FdUtility.h"

PizzaChain::PizzaChain( FdNode* part, PatchNode* panel, QString id )
	:FdGraph(id)
{
	// type
	properties["type"] = "pizza";

	// clone parts
	mPart = (FdNode*)part->clone();
	mPanel = (PatchNode*)panel->clone();
	mPanel->isCtrlPanel = true;

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel);

	// detect hinges
	hinges = detectHinges(part, panel);
}

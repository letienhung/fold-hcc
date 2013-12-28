#include "SandwichChain.h"
#include "FdUtility.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2, QString id )
	:FdGraph(id)
{
	// type
	properties["type"] = "sandwich";

	// clone parts
	mPart = (FdNode*)part->clone();
	mPanel1 = (PatchNode*)panel1->clone();
	mPanel2 = (PatchNode*)panel2->clone();
	mPanel1->isCtrlPanel = true;
	mPanel2->isCtrlPanel = true;

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel1);
	Structure::Graph::addNode(mPanel2);

	// detect hinges
	hinges = detectHinges(mPart, mPanel1);
}

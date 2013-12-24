#include "SandwichChain.h"

SandwichChain::SandwichChain( FdNode* part, FdNode* panel1, FdNode* panel2, QString id )
	:FdGraph(id)
{
	mPart = (FdNode*)part->clone();
	mPanel1 = (FdNode*)panel1->clone();
	mPanel2 = (FdNode*)panel2->clone();
	mPanel1->isCtrlPanel = true;
	mPanel2->isCtrlPanel = true;

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel1);
	Structure::Graph::addNode(mPanel2);
}

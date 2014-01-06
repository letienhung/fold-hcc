#include "PizzaChain.h"
#include "Hinge.h"
#include "Numeric.h"

PizzaChain::PizzaChain( FdNode* part, PatchNode* panel )
	:ChainGraph(part, panel, NULL)
{
	// type
	properties["type"] = "pizza";

	// create hinge links
	nbRods = 1;
	QVector<FdLink*> links;
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i];
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts[0], mPanels[0], 
			jseg.P0, upV,  rV, axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts[0], mPanels[0], 
			jseg.P1, upV, -rV, -axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts[0], mPanels[0], hingeR);
		FdLink* linkL = new FdLink(mParts[0], mPanels[0], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}

	hingeLinks << links;
}

Geom::SectorCylinder PizzaChain::getFoldingVolume( FoldingNode* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	Geom::Segment axisSeg = rootJointSegs[jidx];
	Vector3 rightV = rootRightVs[jidx];
	if (hidx % 2) rightV *= -1;

	return Geom::SectorCylinder(axisSeg, chainUpSeg, rightV);
}

void PizzaChain::prepareFolding( FoldingNode* fn )
{
	// activate corresponding hinges
	foreach(FdLink* l, hingeLinks[0])
		l->properties["active"] = false;

	activeLinks.clear();
	FdLink* activeLink = hingeLinks[0][fn->hingeIdx];
	activeLink->properties["active"] = true;
	activeLinks.push_back(activeLink);
}

void PizzaChain::fold( double t )
{
	// set up fixed and free nodes
	mPanels[0]->properties["fixed"] = true;
	mParts[0]->properties["fixed"] = false;

	// hinge angle
	activeLinks[0]->hinge->setAngleByTime(t);

	// apply folding
	restoreConfiguration();
}

#include "PizzaChain.h"
#include "Hinge.h"

PizzaChain::PizzaChain( FdNode* part, PatchNode* panel )
	:ChainGraph(part, panel)
{
	// type
	properties["type"] = "pizza";

	nbRods = 1;
}

Geom::SectorCylinder PizzaChain::getFoldingVolume( FoldingNode* fn )
{
	Geom::Segment axisSeg = hingeSegs[fn->hingeIdx];
	Vector3 rightV = rightVs[fn->hingeIdx];
	if (fn->direct == FD_LEFT) rightV *= -1;

	return Geom::SectorCylinder(axisSeg, upSeg, rightV);
}

void PizzaChain::fold( FoldingNode* fn )
{
	// clear links
	clearLinks();

	// create new links
	Geom::Segment hs = hingeSegs[fn->hingeIdx];
	Vector3 rV = rightVs[fn->hingeIdx];
	Hinge *hinge = (fn->direct == FD_RIGHT) ?
		new Hinge(mPart, mPanel1, hs.P0, upSeg.Direction,  rV, hs.Direction, hs.length()) :
		new Hinge(mPart, mPanel1, hs.P0, -rV, upSeg.Direction, hs.Direction, hs.length());
	Graph::addLink(new FdLink(mPart, mPanel1, hinge));

	// fold by fixing
	mPanel1->properties["fixed"] = true;
	mPart->properties["fixed"] = false;
	hinge->setState(Hinge::FOLDED);

	restoreConfiguration();
}
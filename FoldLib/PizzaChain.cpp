#include "PizzaChain.h"


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

}
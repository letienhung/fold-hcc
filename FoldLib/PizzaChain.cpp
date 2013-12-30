#include "PizzaChain.h"


PizzaChain::PizzaChain( FdNode* part, PatchNode* panel )
	:ChainGraph(part, panel)
{
	// type
	properties["type"] = "pizza";

	// nbRods
	nbRods = 1;
}

Geom::SectorCylinder PizzaChain::getFoldingVolume( FoldingNode* fn )
{
	Hinge& hinge = (fn->direct == FD_RIGHT) ? 
					mLink1->hinges[0] : mLink1->hinges[1];

	return Geom::SectorCylinder(hinge.Origin, 
		hinge.hX, hinge.hY, hinge.hZ, hinge.zExtent, mLength);
}

void PizzaChain::fold( FoldingNode* fn )
{

}
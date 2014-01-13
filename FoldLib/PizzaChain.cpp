#include "PizzaChain.h"
#include "Hinge.h"
#include "Numeric.h"

PizzaChain::PizzaChain( FdNode* part, PatchNode* panel )
	:ChainGraph(part, panel, NULL)
{
	// type
	properties["type"] = "pizza";

	// create chain
	createChain(1);
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

void PizzaChain::resolveCollision( FoldingNode* fn )
{
	Geom::SectorCylinder sfVolume = fn->properties["sfVolume"].value<Geom::SectorCylinder>();

	// impossible splits
	// to do: resolve collision cooperatively
	if (sfVolume.Radius <= 0) return;  

	int nbPart = ceil( getLength() / (sfVolume.Radius + ZERO_TOLERANCE_LOW) );
	if (nbPart != mParts.size()) createChain(nbPart);
}
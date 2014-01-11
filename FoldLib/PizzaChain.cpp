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
	// change the chain if need
	if (fn->properties.contains("fVolume"))
	{
		Geom::SectorCylinder fVolume0 = getFoldingVolume(fn);
		Geom::SectorCylinder fVolume1 = fn->properties["fVolume"].value<Geom::SectorCylinder>();
		double radius0 = fVolume0.Radius;
		double radius1 = fVolume1.Radius;
		double ratio = (radius0 + ZERO_TOLERANCE_LOW) / radius1;
		int nbPart = ceil(ratio);
		if (nbPart != mParts.size())
			createChain(nbPart);
	}
}
#include "TChain.h"
#include "Hinge.h"
#include "Numeric.h"

TChain::TChain( FdNode* part, PatchNode* panel )
	:ChainGraph(part, panel, NULL)
{
	// type
	properties["type"] = "pizza";

	// create chain
	createChain(1);
}

Geom::SectorCylinder TChain::getFoldingVolume( FoldingNode* fn )
{
	// hinge axis and rightV
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	return Geom::SectorCylinder(axisSeg, chainUpSeg, rightV);
}

void TChain::resolveCollision( FoldingNode* fn )
{
	Geom::SectorCylinder sfVolume = fn->properties["sfVolume"].value<Geom::SectorCylinder>();

	// impossible splits
	// to do: resolve collision cooperatively
	if (sfVolume.Radius <= 0) return;  

	int nbPart = ceil( getLength() / (sfVolume.Radius + ZERO_TOLERANCE_LOW) );
	if (nbPart != mParts.size()) createChain(nbPart);
}

QVector<FoldingNode*> TChain::generateFoldOptions()
{
	return QVector<FoldingNode*>();
}

void TChain::modify( FoldingNode* fn )
{

}

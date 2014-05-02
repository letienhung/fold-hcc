#include "TChain.h"
#include "Hinge.h"
#include "Numeric.h"

TChain::TChain( PatchNode* master, FdNode* slave)
	:ChainGraph(slave, master, NULL)// ChainGraph has cloned slave and master
{
	// type
	mType = T_CHAIN;
}

Geom::SectorCylinder TChain::getFoldingVolume( FoldOption* fn )
{
	// hinge axis and rightV
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	return Geom::SectorCylinder(axisSeg, chainUpSeg, rightV);
}

void TChain::resolveCollision( FoldOption* fn )
{
	Geom::SectorCylinder sfVolume = fn->properties["sfVolume"].value<Geom::SectorCylinder>();

	// impossible splits
	// to do: resolve collision cooperatively
	if (sfVolume.Radius <= 0) return;  

	int nbPart = ceil( getLength() / (sfVolume.Radius + ZERO_TOLERANCE_LOW) );
	if (nbPart != mParts.size()) createChain(nbPart);
}

QVector<FoldOption*> TChain::generateFoldOptions()
{
	QVector<FoldOption*> options;



	return options;
}

void TChain::modify( FoldOption* fn )
{

}

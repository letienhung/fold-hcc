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
	// shrink hinge axis
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg = rootJointSegs[hidx];
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	axisSeg.cropRange01(t0, t1);

	// chain up segment
	Geom::Segment upseg = chainUpSeg;
	upseg.cropRange01(0, 1.0/fn->nbsplit);

	// rightV
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
	QVector<FoldOption*> options = ChainGraph::generateFoldOptions(0, 1, 3);

	// fold volume
	foreach (FoldOption* fn, options)
	{
		Geom::SectorCylinder fVolume = this->getFoldingVolume(fn);
		fn->properties["fVolume"].setValue(fVolume);
	}

	return options;
}

void TChain::applyFoldOption( FoldOption* fn )
{

}

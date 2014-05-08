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
	int hidx = fn->jointAxisIdx;
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

	// fold volume
	Geom::SectorCylinder fv(axisSeg, chainUpSeg, rightV);
	fv.shrinkEpsilon();
	return fv;
}

QVector<FoldOption*> TChain::generateFoldOptions()
{
	QVector<FoldOption*> options = ChainGraph::generateFoldOptions(0, 0, 1);

	// fold volume
	foreach (FoldOption* fn, options)
	{
		Geom::SectorCylinder fVolume = this->getFoldingVolume(fn);
		fn->properties["fVolume"].setValue(fVolume);
	}

	return options;
}

QVector<Geom::Plane> TChain::generateCutPlanes( int nbSplit )
{
	// plane of master
	Geom::Plane master = mMasters[0]->mPatch.getPlane();
	if (master.whichSide(mOrigSlave->center()) < 0) master.flip();

	// deltaV to shift up
	double step = getLength() / (nbSplit + 1);

	QVector<Geom::Plane> cutPlanes;
	for (int i = 0; i < nbSplit; i++)
	{
		Vector3 deltaV = (i+1) * step * master.Normal;
		cutPlanes << master.translated(deltaV);
	}

	return cutPlanes;
}

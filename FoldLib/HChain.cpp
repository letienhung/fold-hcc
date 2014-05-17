#include "HChain.h"
#include "Numeric.h"

HChain::HChain( FdNode* slave, PatchNode* master_low, PatchNode* master_high)
	:ChainGraph(slave, master_low, master_high)
{
}

Geom::Rectangle HChain::getFoldRegion(FoldOption* fn)
{
	// axis and rightV
	int hidx = fn->jointAxisIdx;
	Geom::Segment axisSeg1 = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	// shrink along axisSeg
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	axisSeg1.cropRange01(t0, t1);
	Geom::Segment axisSeg2 = axisSeg1;

	// shift the axis along rightV
	double width = getLength() / (fn->nbsplit + 1);
	axisSeg2.translate(width * rightV);

	// shrink epsilon
	Geom::Rectangle rect(QVector<Vector3>() 
		<< axisSeg1.P0	<< axisSeg1.P1
		<< axisSeg2.P1  << axisSeg2.P0 );
	rect.scale(1 - ZERO_TOLERANCE_LOW);
	return rect;
}


Geom::Rectangle HChain::getMaxFoldRegion( bool right )
{
	FoldOption fn(0, right, 1.0, 0.0, 1, "");
	return getFoldRegion(&fn);
}


QVector<FoldOption*> HChain::generateFoldOptions()
{
	QVector<FoldOption*> options = ChainGraph::generateFoldOptions(1, 1, 2);

	// fold area
	foreach (FoldOption* fn, options)
	{
		Geom::Rectangle fArea = this->getFoldRegion(fn);
		fn->properties["fArea"].setValue(fArea);
	}

	return options;
}

// N is the number of cut planes
QVector<Geom::Plane> HChain::generateCutPlanes( int N )
{
	// plane of master0
	Geom::Plane master0 = mMasters[0]->mPatch.getPlane();
	if (master0.whichSide(mOrigSlave->center()) < 0) master0.flip();

	// deltaV to shift up
	double step = getLength() / (N + 1);

	QVector<Geom::Plane> cutPlanes;
	for (int i = 0; i < N; i++)
	{
		Vector3 deltaV = (i+1) * step * master0.Normal;
		cutPlanes << master0.translated(deltaV);
	}

	// to-do: is N is even, cutting will be different

	return cutPlanes;
}

PatchNode* HChain::getTopMaster()
{
	return mMasters[1];
}

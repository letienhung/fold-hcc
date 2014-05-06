#include "HChain.h"
#include "Numeric.h"

HChain::HChain( FdNode* slave, PatchNode* master1, PatchNode* master2)
	:ChainGraph(slave, master1, master2)
{
	// type
	properties["type"] = "sandwich";
}

Geom::Rectangle2 HChain::getFoldRegion(FoldOption* fn)
{
	// axis and rightV
	int hidx = fn->hingeIdx;
	Geom::Segment axisSeg1 = rootJointSegs[hidx];
	Vector3 rightV = rootRightVs[hidx];
	if (!fn->rightSide) rightV *= -1;

	// shrink along axisSeg
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	axisSeg1.cropRange01(t0, t1);
	Geom::Segment axisSeg2 = axisSeg1;

	// shift the axis along rightV
	if (fn->nbsplit == 1)
	{
		double width = getLength() * 0.5;
		axisSeg2.translate(width * rightV);
	}
	else if (fn->nbsplit == 2)
	{
		double width = getLength() * 0.25;
		axisSeg1.translate(-width * rightV);
		axisSeg2.translate( width * rightV);
	}

	// conners of projected rectangle
	QVector<Vector2> conners;
	Geom::Rectangle& panel_rect = mMasters[0]->mPatch;
	conners << panel_rect.getProjCoordinates(axisSeg1.P0) 
		<< panel_rect.getProjCoordinates(axisSeg1.P1) 
		<< panel_rect.getProjCoordinates(axisSeg2.P1) 
		<< panel_rect.getProjCoordinates(axisSeg2.P0);

	return Geom::Rectangle2(conners);
}

QVector<FoldOption*> HChain::generateFoldOptions()
{
	QVector<FoldOption*> options = ChainGraph::generateFoldOptions(1, 1, 2);

	// fold area
	foreach (FoldOption* fn, options)
	{
		Geom::Rectangle2 fArea = this->getFoldRegion(fn);
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

	// to-do: is N is odd, cutting will be different

	return cutPlanes;
}

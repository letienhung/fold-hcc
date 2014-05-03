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

Geom::Segment2 HChain::getFoldingAxis2D( FoldOption* fn )
{
	Geom::Segment axisSeg = getJointSegment(fn);
	Geom::Rectangle& panel_rect = mMasters[0]->mPatch;

	return panel_rect.get2DSegment(axisSeg);
}

Geom::Segment HChain::getJointSegment( FoldOption* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	return rootJointSegs[jidx];
}

QVector<FoldOption*> HChain::generateFoldOptions()
{
	QVector<FoldOption*> options = ChainGraph::generateFoldOptions(1, 2, 3);

	// fold area
	foreach (FoldOption* fn, options)
	{
		Geom::Rectangle2 fArea = this->getFoldRegion(fn);
		fn->properties["fArea"].setValue(fArea);
	}

	return options;
}

void HChain::modify( FoldOption* fn )
{

}


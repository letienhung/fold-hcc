#include "SandwichChain.h"
#include "Numeric.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	createChain(2);
}

Geom::Rectangle2 SandwichChain::getFoldingArea(FoldingNode* fn)
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	Geom::Segment axisSeg = rootJointSegs[jidx];
	Vector3 rightV = rootRightVs[jidx];
	if (hidx % 2) rightV *= -1;

	double width = chainUpSeg.length() / mParts.size();
	Geom::Segment seg = axisSeg;
	seg.translate(width * rightV);

	QVector<Vector2> conners;
	Geom::Rectangle& panel_rect = mPanels[0]->mPatch;
	conners << panel_rect.getProjCoordinates(axisSeg.P0) 
		<< panel_rect.getProjCoordinates(axisSeg.P1) 
		<< panel_rect.getProjCoordinates(seg.P1) 
		<< panel_rect.getProjCoordinates(seg.P0);

	return Geom::Rectangle2(conners);
}

void SandwichChain::resolveCollision( FoldingNode* fn )
{

}

Geom::Segment2 SandwichChain::getFoldingAxis2D( FoldingNode* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	Geom::Segment axisSeg = rootJointSegs[jidx];
	Geom::Rectangle& panel_rect = mPanels[0]->mPatch;

	Vector2 p0 = panel_rect.getProjCoordinates(axisSeg.P0);
	Vector2 p1 = panel_rect.getProjCoordinates(axisSeg.P1);

	return Geom::Segment2(p0, p1);
}

Geom::Segment SandwichChain::getJointSegment( FoldingNode* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	return rootJointSegs[jidx];
}

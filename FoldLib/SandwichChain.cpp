#include "SandwichChain.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	nbRods = 2;
}

Geom::Rectangle2 SandwichChain::getFoldingArea(FoldingNode* fn)
{
	Geom::Segment axisSeg = hingeSegs[fn->hingeIdx];
	Vector3 rightV = rightVs[fn->hingeIdx];
	if (fn->direct == FD_LEFT) rightV *= -1;

	double width = upSeg.length() / nbRods;
	Geom::Segment seg = axisSeg;
	seg.translate(width * rightV);

	QVector<Vector2> conners;
	Geom::Rectangle& panel_rect = mPanel1->mPatch;
	conners << panel_rect.getProjCoordinates(axisSeg.P0) 
		<< panel_rect.getProjCoordinates(axisSeg.P1) 
		<< panel_rect.getProjCoordinates(seg.P1) 
		<< panel_rect.getProjCoordinates(seg.P0);

	return Geom::Rectangle2(conners);
}

void SandwichChain::fold( FoldingNode* fn )
{

}

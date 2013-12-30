#include "SandwichChain.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	// nbRods
	nbRods = 2;
}

Geom::Rectangle2 SandwichChain::getFoldingArea(FoldingNode* fn)
{
	Hinge& hinge = (fn->direct == FD_RIGHT) ? 
		mLink1->hinges[0] : mLink1->hinges[1];

	Geom::Segment axisSeg();
	Vector3 v2 = v2s[fn->hingeIdx];
	if (fn->direct == FD_LEFT) v2 *= -1;

	double width = r1.length() / nbFold;
	Geom::Segment seg = axisSeg;
	seg.translate(width * v2);

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

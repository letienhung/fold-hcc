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

	double width = getLength() / mParts.size();
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

Geom::Segment2 SandwichChain::getFoldingAxis2D( FoldingNode* fn )
{
	Geom::Segment axisSeg = getJointSegment(fn);
	Geom::Rectangle& panel_rect = mPanels[0]->mPatch;

	return panel_rect.get2DSegment(axisSeg);
}

Geom::Segment SandwichChain::getJointSegment( FoldingNode* fn )
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	return rootJointSegs[jidx];
}


void SandwichChain::resolveCollision( FoldingNode* fn )
{
	// folding areas
	Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();
	Geom::Rectangle2 sfArea = fn->properties["sfArea"].value<Geom::Rectangle2>();
	Geom::Segment2 axisSeg = getFoldingAxis2D(fn);

	// length (extent perp to axis seg) of folding areas
	double length = fArea.getPerpExtent(axisSeg.Direction);
	double slength = sfArea.getPerpExtent(axisSeg.Direction);

	// impossible splits
	// to do: resolve collision cooperatively
	if (slength <= 0) return; 

	// split
	int nbPart = ceil( 2 * length / (slength + ZERO_TOLERANCE_LOW) );
	if (nbPart % 2) nbPart++;
	if (nbPart != mParts.size()) createChain(nbPart);

	// shrink along axisSeg
	int aid = sfArea.getAxisId(axisSeg.Direction);
	Vector2 sEnd0 = sfArea.getEdgeCenter(aid, false);
	Vector2 sEnd1 = sfArea.getEdgeCenter(aid, true);
	Vector2 sEnd0Coord = fArea.getCoordinates(sEnd0);
	Vector2 sEnd1Coord = fArea.getCoordinates(sEnd1);
	double t0 = sEnd0Coord[aid];
	double t1 = sEnd1Coord[aid];
	if (t1 - t0 < 2 - ZERO_TOLERANCE_LOW)
	{
		shrinkChainAlongJoint(t0, t1);
		resetHingeLinks();
	}
}


#include "SandwichChain.h"
#include "FdUtility.h"
#include "RodNode.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:FdGraph(part->mID)
{
	// type
	properties["type"] = "sandwich";

	// clone parts
	mPart = (FdNode*)part->clone();
	mPanel1 = (PatchNode*)panel1->clone();
	mPanel2 = (PatchNode*)panel2->clone();
	mPanel1->isCtrlPanel = true;
	mPanel2->isCtrlPanel = true;

	Structure::Graph::addNode(mPart);
	Structure::Graph::addNode(mPanel1);
	Structure::Graph::addNode(mPanel2);

	// detect hinges connecting panel-1
	hinges = detectHinges(mPart, mPanel1);

	// number of folds
	nbFold = 2;

	// r1
	Geom::Segment axisSeg = hinges[0];
	Vector3 origin = axisSeg.P0;
	if (mPart->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mPart;
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(axisSeg.Direction);
		r1 = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mPart;
		r1 = partRod->mRod;
	}
	if (r1.getProjCoordinates(origin) > 0) r1.flip();

	// v2
	foreach (Geom::Segment hinge, hinges)
	{
		Vector3 crossAxisV1 = cross(axisSeg.Direction, r1.Direction);
		Vector3 v2 = mPanel1->mPatch.getProjectedVector(crossAxisV1);

		v2s.push_back(v2);
	}
}

Geom::Rectangle2 SandwichChain::getFoldingArea(FoldingNode* fn)
{
	Geom::Segment axisSeg = hinges[fn->hingeIdx];
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

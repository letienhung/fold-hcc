#include "SandwichChain.h"
#include "Numeric.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	// split part
	Vector3 pcenter = (mPanels[0]->center() + mPanels[1]->center()) * 0.5;
	Vector3 pnormal = mPanels[0]->mPatch.Normal;
	Geom::Plane cutPlane(pcenter, pnormal);
	mParts = split(mParts[0], cutPlane, 0);
	sortParts();

	// create hinge links between mPanels[0] and mParts[0]
	QVector<FdLink*> links;
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i];
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts[0], mPanels[0], 
			jseg.P0, upV,  rV, axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts[0], mPanels[0], 
			jseg.P1, upV, -rV, -axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts[0], mPanels[0], hingeR);
		FdLink* linkL = new FdLink(mParts[0], mPanels[0], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}

	hingeLinks << links;
	links.clear();

	// create hinge links between two rods in the chain
	nbRods = 2;
	double step = 2.0 / nbRods;
	for (int i = 1; i < nbRods; i++) // each joint
	{
		Vector3 pos = chainUpSeg.getPosition(-1 + step * i);
		Vector3 deltaV =  pos - chainUpSeg.P0;

		FdNode* part1 = mParts[i];
		FdNode* part2 = mParts[i-1];

		// create a pair of links for each joint seg
		for (int j = 0; j < rootJointSegs.size(); j++)
		{
			Geom::Segment jseg = rootJointSegs[j].translated(deltaV);
			Vector3 upV = chainUpSeg.Direction;
			Vector3 axisV = jseg.Direction;
			Hinge* hingeR = new Hinge(part1, part2, 
				jseg.P0, upV, -upV, axisV, jseg.length());
			Hinge* hingeL = new Hinge(part1, part2, 
				jseg.P1, upV, -upV, -axisV, jseg.length());

			FdLink* linkR = new FdLink(part1, part2, hingeR);
			FdLink* linkL = new FdLink(part1, part2, hingeL);

			links << linkR << linkL;

			Graph::addLink(linkR);
			Graph::addLink(linkL);
		}

		hingeLinks << links;
		links.clear();
	}

	// create hinge links between mPanels[1] and mParts[1]
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i].translated(chainUpSeg.P1 - chainUpSeg.P0);
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts.last(), mPanels[1], 
			jseg.P1, -upV,  rV, -axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts.last(), mPanels[1], 
			jseg.P0, -upV, -rV, axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts.last(), mPanels[1], hingeR);
		FdLink* linkL = new FdLink(mParts.last(), mPanels[1], hingeL);
		links.clear();
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}

	hingeLinks << links;
}

void SandwichChain::sortParts()
{
	QMap<double, FdNode*> distPartMap;

	Geom::Plane panel_plane = mPanels[0]->mPatch.getPlane();
	foreach(FdNode* n, mParts)
	{
		double dist = panel_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}

	mParts = distPartMap.values().toVector();
}

Geom::Rectangle2 SandwichChain::getFoldingArea(FoldingNode* fn)
{
	int hidx = fn->hingeIdx;
	int jidx = hidx / 2;
	Geom::Segment axisSeg = rootJointSegs[jidx];
	Vector3 rightV = rootRightVs[jidx];
	if (hidx % 2) rightV *= -1;

	double width = chainUpSeg.length() / nbRods;
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

void SandwichChain::prepareFolding( FoldingNode* fn )
{
	activeLinks.clear();

	int hidx_m = fn->hingeIdx;
	int hidx_v = (hidx_m % 2) ? hidx_m - 1 : hidx_m + 1;
	for (int i = 0; i < hingeLinks.size(); i++)
	{
		// activate corresponding hinges
		foreach(FdLink* l, hingeLinks[i])
			l->properties["active"] = false;

		int hidx = (i % 2) ? hidx_v : hidx_m;
		FdLink* activeLink = hingeLinks[i][hidx];
		activeLink->properties["active"] = true;
		activeLinks << activeLink;
	}
}



void SandwichChain::fold( double t )
{
	// fix panels[0] but free all others
	foreach (Structure::Node* n, nodes)
		n->properties["fixed"] = false;
	mPanels[0]->properties["fixed"] = true;

	// hinge angle
	foreach(FdLink* alink, activeLinks)
		alink->hinge->setAngleByTime(t);

	// apply folding
	restoreConfiguration();
}

#include "SandwichChain.h"
#include "Numeric.h"

SandwichChain::SandwichChain( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	:ChainGraph(part, panel1, panel2)
{
	// type
	properties["type"] = "sandwich";

	splitChain(2);
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

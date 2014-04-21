#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "mcqd.h"

SandwichLayer::SandwichLayer( QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, 
	QString id, Geom::Box &bBox )
	:LayerGraph(parts, panel1, panel2, id, bBox)
{
	mType = LayerGraph::SANDWICH;

	mPanel1 = (PatchNode*) getNode(panel1->mID);
	mPanel2 = (PatchNode*) getNode(panel2->mID);
	mPanel1->properties["isCtrlPanel"] = true;
	mPanel2->properties["isCtrlPanel"] = true;

	// create chains
	double thr = mPanel1->mBox.getExtent(mPanel1->mPatch.Normal) * 2;
	QVector<FdNode*> panels;
	panels << mPanel1 << mPanel2;

	foreach (FdNode* n, getFdNodes())
	{
		if (n->properties.contains("isCtrlPanel")) continue;

		if (getDistance(n, panels) < thr)
		{
			chains.push_back(new SandwichChain(n, mPanel1, mPanel2));
		}
	}

	// allocate space for solution
	foldSolution.resize(chains.size());
}

void SandwichLayer::foldabilize()
{
	buildCollisionGraph();

	// get all folding nodes
	QVector<FoldingNode*> fns = fog->getAllFoldingNodes();

	// the adjacent matrix
	QVector<bool> dumpy(fns.size(), false);
	QVector<QVector<bool>> conn(fns.size(), dumpy);
	for (int i = 0; i < fns.size(); i++)
	{
		FoldingNode* fn = fns[i];
		for (int j = i+1; j < fns.size(); j++)
		{
			FoldingNode* other_fn = fns[j];

			// connect siblings and colliding folding options
			if (fog->areSiblings(fn->mID, other_fn->mID) ||
				fog->verifyLinkType(fn->mID, other_fn->mID, "collision"))
			{
				conn[i][j] = true;
				conn[j][i] = true;
			}
		}
	}

	// maximum independent sets (maximum clique on the dual graph)
	// get the dual graph
	bool** dual_conn = new bool*[fns.size()];
	for (int i = 0; i < fns.size(); i++)
	{
		dual_conn[i] = new bool[fns.size()];
		for (int j = 0; j < fns.size(); j++)
		{
			// do not touch diagonal entries
			if (i == j) continue;
			dual_conn[i][j] = !conn[i][j];
		}
	}

	// maximum clique on the dual graph
	Maxclique m(dual_conn, fns.size());
	int* qmax;
	int qsize;
	m.mcq(qmax, qsize);

	// delete matrix
	for (int i = 0; i < fns.size(); i++)
		delete [] dual_conn[i];
	delete [] dual_conn;

	// visualize the maximum independent set
	for (int i = 0; i< qsize; i++)
	{
		int fnidx = qmax[i];
		fns[fnidx]->addTag("selected");
	}
}

void SandwichLayer::buildCollisionGraph()
{
	// clear
	fog->clear();

	// nodes and folding links
	for(int i = 0; i < chains.size(); i++)
	{
		SandwichChain* chain = (SandwichChain*)chains[i];

		// chain nodes
		ChainNode* cn = new ChainNode(i, chain->mID);
		fog->addNode(cn);

		for (int j = 0; j < chain->rootJointSegs.size(); j++)
		{
			// folding nodes
			QString fnid = chain->mID + "_" + QString::number(j);
			// left
			QString fnid1 = fnid + "_" + QString::number(false);
			FoldingNode* fn1 = new FoldingNode(j, false, fnid1);
			Geom::Rectangle2 fArea1 = chain->getFoldingArea(fn1);
			fn1->properties["fArea"].setValue(fArea1);
			fog->addNode(fn1);
			// right
			QString fnid2 = fnid + "_" + QString::number(true);
			FoldingNode* fn2 = new FoldingNode(j, true, fnid2);
			Geom::Rectangle2 fArea2 = chain->getFoldingArea(fn2);
			fn2->properties["fArea"].setValue(fArea2);
			fog->addNode(fn2);

			// folding links
			fog->addFoldingLink(cn, fn1);
			fog->addFoldingLink(cn, fn2);

			// debug
			Geom::Rectangle rect1 = mPanel1->mPatch.getRectangle(fArea1);
			Geom::Rectangle rect2 = mPanel1->mPatch.getRectangle(fArea2);
			QVector<Geom::Segment> segs;
			segs += rect1.getEdgeSegments();
			segs += rect2.getEdgeSegments();
			addDebugSegments(segs);
		}
	}

	// barrier nodes
	QVector<Geom::Rectangle> barriers = barrierBox.getFaceRectangles();
	Vector3 pNormal = mPanel1->mPatch.Normal;
	for (int i = 0; i < Geom::Box::NB_FACES; i++)
	{
		if (fabs(dot(pNormal, barriers[i].Normal)) > 0.5)
			continue;
		fog->addNode(new BarrierNode(i));
	}

	// collision links
	QVector<FoldingNode*> fns = fog->getAllFoldingNodes();
	for (int i = 0; i < fns.size(); i++)
	{
		FoldingNode* fn = fns[i];
		ChainNode* cn = fog->getChainNode(fn->mID);
		Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();

		// with barriers
		QVector<Vector3> fConners = mPanel1->mPatch.getRectangle(fArea).getConners();
		foreach (BarrierNode* bn, fog->getAllBarrierNodes())
		{
			Geom::Plane bplane = barriers[bn->faceIdx].getPlane();
			if (!bplane.onSameSide(fConners))
			{
				fog->addCollisionLink(fn, bn);
			}
		}

		// with other folding nodes
		for (int j = i+1; j < fns.size(); j++)
		{
			FoldingNode* other_fn = fns[j];

			// skip siblings
			if (fog->areSiblings(fn->mID, other_fn->mID)) continue; 

			Geom::Rectangle2 other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle2>();
			if (Geom::IntrRect2Rect2::test(fArea, other_fArea))
			{
				fog->addCollisionLink(fn, other_fn);
			}
		}
	}
}

QVector<Structure::Node*> SandwichLayer::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// chain parts
	// fold all chains simultaneously
	for (int i = 0; i < chains.size(); i++)
		knodes += chains[i]->getKeyframeParts(t);

	// control panels
	if (chains.isEmpty())
		// empty layer: panel is the only part
		knodes += nodes.front()->clone(); 
	else
		// layer with chains: get panels from first chain
		knodes += chains.front()->getKeyFramePanels(t);

	return knodes;
}
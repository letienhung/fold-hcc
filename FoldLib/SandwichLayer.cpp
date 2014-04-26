#include "SandwichLayer.h"
#include "SandwichChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "CliquerAdapter.h"

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
}

void SandwichLayer::foldabilize()
{
	buildCollisionGraph();

	// get all folding nodes
	QVector<FoldingNode*> fns = fog->getAllFoldingNodes();

	// the dual adjacent matrix
	QVector<bool> dumpy(fns.size(), true);
	QVector< QVector<bool> > conn(fns.size(), dumpy);
	for (int i = 0; i < fns.size(); i++)
	{
		// the i-th node
		FoldingNode* fn = fns[i];

		// diagonal entry
		conn[i][i] = false;

		// other fold options
		for (int j = i+1; j < fns.size(); j++)
		{
			// the j-th node
			FoldingNode* other_fn = fns[j];

			// connect siblings and colliding folding options
			if (fog->areSiblings(fn->mID, other_fn->mID) ||
				fog->verifyLinkType(fn->mID, other_fn->mID, "collision"))
			{
				conn[i][j] = false;
				conn[j][i] = false;
			}
		}
	}

	// find minimum cost maximum cliques
	QVector<double> weights;
	foreach(FoldingNode* fn, fns) weights.push_back(fn->getCost());
	CliquerAdapter cliquer(conn, weights);
	cliquer.computeWeightsOfAllMaxCliques();
	QVector<int> q = cliquer.getMinWeightMaxClique();

	// fold solution
	foreach(int idx, q)
		foldSolution.push_back(fns[idx]);

	// apply modification

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

		// folding nodes and links
		QVector<Geom::Segment> segs; // debug
		foreach(FoldingNode* fn, chain->generateFoldOptions())
		{
			fog->addNode(fn);
			fog->addFoldingLink(cn, fn);

			// debug
			Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();
			Geom::Rectangle rect = mPanel1->mPatch.getRectangle(fArea);
			segs += rect.getEdgeSegments();
		}
		addDebugSegments(segs);	
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
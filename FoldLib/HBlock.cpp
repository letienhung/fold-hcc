#include "HBlock.h"
#include "HChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "CliquerAdapter.h"

HBlock::HBlock( QVector<PatchNode*>& masters, QVector<FdNode*>& slaves,  
				QVector< QVector<int> >& masterPairs, QString id )
				:BlockGraph(id)
{
	// type
	mType = BlockGraph::H_BLOCK;

	// clone nodes
	foreach (PatchNode* m, masters) Structure::Graph::addNode(m->clone());
	foreach (FdNode* s, slaves) Structure::Graph::addNode(s->clone());

	// create chains
	for (int i = 0; i < slaves.size(); i++)
	{
		int midx1 = masterPairs[i].first();
		int midx2 = masterPairs[i].last();
		chains << new HChain(slaves[i], masters[midx1], masters[midx2]);
	}
}

void HBlock::foldabilize()
{
	//buildCollisionGraph();

	//// get all folding nodes
	//QVector<FoldingNode*> fns = fog->getAllFoldingNodes();

	//// the dual adjacent matrix
	//QVector<bool> dumpy(fns.size(), true);
	//QVector< QVector<bool> > conn(fns.size(), dumpy);
	//for (int i = 0; i < fns.size(); i++)
	//{
	//	// the i-th node
	//	FoldingNode* fn = fns[i];

	//	// diagonal entry
	//	conn[i][i] = false;

	//	// other fold options
	//	for (int j = i+1; j < fns.size(); j++)
	//	{
	//		// the j-th node
	//		FoldingNode* other_fn = fns[j];

	//		// connect siblings and colliding folding options
	//		if (fog->areSiblings(fn->mID, other_fn->mID) ||
	//			fog->verifyLinkType(fn->mID, other_fn->mID, "collision"))
	//		{
	//			conn[i][j] = false;
	//			conn[j][i] = false;
	//		}
	//	}
	//}

	//// find minimum cost maximum cliques
	//QVector<double> weights;
	//foreach(FoldingNode* fn, fns) weights.push_back(fn->getCost());
	//CliquerAdapter cliquer(conn, weights);
	//cliquer.computeWeightsOfAllMaxCliques();
	//QVector<int> q = cliquer.getMinWeightMaxClique();

	//// fold solution
	//foldSolution.clear();
	//foldSolution.resize(chains.size());
	//foreach(int idx, q)
	//{
	//	FoldingNode* fn = fns[idx];
	//	ChainNode* cn = fog->getChainNode(fn->mID);
	//	foldSolution[cn->chainIdx] = fn;
	//}

	//// apply modification
	//for (int i = 0; i < chains.size(); i++)
	//{
	//	chains[i]->modify(foldSolution[i]);
	//}
}

void HBlock::buildCollisionGraph()
{
	//// clear
	//fog->clear();

	//// nodes and folding links
	//for(int i = 0; i < chains.size(); i++)
	//{
	//	HChain* chain = (HChain*)chains[i];

	//	// chain nodes
	//	ChainNode* cn = new ChainNode(i, chain->mID);
	//	fog->addNode(cn);

	//	// folding nodes and links
	//	QVector<Geom::Segment> segs; // debug
	//	foreach(FoldingNode* fn, chain->generateFoldOptions())
	//	{
	//		fog->addNode(fn);
	//		fog->addFoldingLink(cn, fn);

	//		// debug
	//		Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();
	//		Geom::Rectangle rect = mPanel1->mPatch.getRectangle(fArea);
	//		segs += rect.getEdgeSegments();
	//	}
	//	addDebugSegments(segs);	
	//}

	//// barrier nodes
	//QVector<Geom::Rectangle> barriers = barrierBox.getFaceRectangles();
	//Vector3 pNormal = mPanel1->mPatch.Normal;
	//for (int i = 0; i < Geom::Box::NB_FACES; i++)
	//{
	//	if (fabs(dot(pNormal, barriers[i].Normal)) > 0.5)
	//		continue;
	//	fog->addNode(new BarrierNode(i));
	//}

	//// collision links
	//QVector<FoldingNode*> fns = fog->getAllFoldingNodes();
	//for (int i = 0; i < fns.size(); i++)
	//{
	//	FoldingNode* fn = fns[i];
	//	ChainNode* cn = fog->getChainNode(fn->mID);
	//	Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();

	//	// with barriers
	//	QVector<Vector3> fConners = mPanel1->mPatch.getRectangle(fArea).getConners();
	//	foreach (BarrierNode* bn, fog->getAllBarrierNodes())
	//	{
	//		Geom::Plane bplane = barriers[bn->faceIdx].getPlane();
	//		if (!bplane.onSameSide(fConners))
	//		{
	//			fog->addCollisionLink(fn, bn);
	//		}
	//	}

	//	// with other folding nodes
	//	for (int j = i+1; j < fns.size(); j++)
	//	{
	//		FoldingNode* other_fn = fns[j];

	//		// skip siblings
	//		if (fog->areSiblings(fn->mID, other_fn->mID)) continue; 

	//		Geom::Rectangle2 other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle2>();
	//		if (Geom::IntrRect2Rect2::test(fArea, other_fArea))
	//		{
	//			fog->addCollisionLink(fn, other_fn);
	//		}
	//	}
	//}
}

QVector<Structure::Node*> HBlock::getKeyFrameNodes( double t )
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
#include "PizzaLayer.h"
#include "PizzaChain.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "SectorCylinder.h"
#include <QDir>


PizzaLayer::PizzaLayer( QVector<FdNode*> nodes, PatchNode* panel, QString id )
	:LayerGraph(nodes, NULL, panel, id)
{
	// type
	mType = LayerGraph::PIZZA;

	// panel
	mPanel = (PatchNode*)getNode(panel->mID);
	mPanel->isCtrlPanel = true;

	// create chains
	double thr = mPanel->mBox.getExtent(mPanel->mPatch.Normal) * 2;
	foreach (FdNode* n, getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		if (getDistance(n, mPanel) < thr)
		{
			chains.push_back(new PizzaChain(n, mPanel));
		}
	}
}


PizzaLayer::~PizzaLayer()
{
	delete dy_graph;
}

void PizzaLayer::buildDependGraph()
{
	// clear
	dy_graph->clear();

	// nodes and folding links
	for(int i = 0; i < chains.size(); i++)
	{
		PizzaChain* chain = (PizzaChain*)chains[i];

		// chain nodes
		ChainNode* cn = new ChainNode(i, chain->mID);
		dy_graph->addNode(cn);


		for (int j = 0; j < chain->rootJointSegs.size(); j++)
		{
			// folding nodes
			QString fnid1 = chain->mID + "_" + QString::number(2*j);
			FoldingNode* fn1 = new FoldingNode(2*j, fnid1);
			dy_graph->addNode(fn1);

			QString fnid2 = chain->mID + "_" + QString::number(2*j+1);
			FoldingNode* fn2 = new FoldingNode(2*j+1, fnid2);
			dy_graph->addNode(fn2);

			// folding links
			dy_graph->addFoldingLink(cn, fn1);
			dy_graph->addFoldingLink(cn, fn2);
		}

	}

	// collision links
	// between folding nodes and other chain nodes
	foreach(FoldingNode* fn, dy_graph->getAllFoldingNodes())
	{
		ChainNode* cn = dy_graph->getChainNode(fn->mID);
		PizzaChain* chain = (PizzaChain*) getChain(cn->mID);
		Geom::SectorCylinder fVolume = chain->getFoldingVolume(fn);

		foreach(ChainNode* other_cn, dy_graph->getAllChainNodes())
		{
			if (cn == other_cn) continue;

			PizzaChain* other_chain = (PizzaChain*) getChain(other_cn->mID);
			FdNode* other_part = other_chain->mParts[0];

			bool collide = false;
			if (other_part->mType == FdNode::PATCH)
			{
				PatchNode* other_patch = (PatchNode*) other_part;
				if (fVolume.intersects(other_patch->mPatch))
					collide = true;
			}else
			{
				RodNode* other_rod = (RodNode*) other_part;
				if (fVolume.intersects(other_rod->mRod))
					collide = true;
			}

			// add collision link
			if (collide)
			{
				dy_graph->addCollisionLink(fn, other_cn);
			}
		}
	}
}

QVector<Structure::Node*> PizzaLayer::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// evenly distribute time among pizza chains
	QVector<double> chainStarts = getEvenDivision(chains.size());

	// chain parts
	for (int i = 0; i < chains.size(); i++)
	{
		double lt = getLocalTime(t, chainStarts[i], chainStarts[i+1]);
		knodes += chains[i]->getKeyframeParts(lt);
	}

	// control panels
	if (chains.isEmpty())
	{
		// empty layer: panel is the only part
		knodes += nodes.front()->clone(); 
	}
	else
	{
		// layer with chains: get panels from first chain
		double lt = getLocalTime(t, chainStarts[0], chainStarts[1]);
		knodes += chains.front()->getKeyFramePanels(lt);
	}

	return knodes;
}
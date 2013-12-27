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
			QString id = "Ch-" + n->mID;
			chains.push_back(new PizzaChain(n, mPanel, id));
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

		for (int j = 0; j < chain->hinges.size(); j++)
		{
			// folding nodes
			QString fnid1 = chain->mID + "_" + QString::number(2*j);
			FoldingNode* fn1 = new FoldingNode(j, FD_RIGHT, fnid1);
			dy_graph->addNode(fn1);

			QString fnid2 = chain->mID + "_" + QString::number(2*j+1);
			FoldingNode* fn2 = new FoldingNode(j, FD_LEFT, fnid2);
			dy_graph->addNode(fn2);

			// folding links
			dy_graph->addFoldingLink(cn, fn1);
			dy_graph->addFoldingLink(cn, fn2);
		}
	}

	// collision links
	for(int i = 0; i < chains.size(); i++)
	{
		PizzaChain* chain = (PizzaChain*)chains[i];
		QVector<FoldingNode*> fns = dy_graph->getFoldingNodes(chain->mID);

		// each other chain
		for (int j = 0; j < chains.size(); j++)
		{
			if (i == j) continue;

			PizzaChain* other_chain = (PizzaChain*)chains[j];
			FdNode* other_part = other_chain->mPart;

			// each folding
			for (int k = 0; k < fns.size(); k++)
			{
				FoldingNode* fn = fns[k];
				Geom::SectorCylinder fVolume = chain->getFoldingVolume(fn);

				// chain(fn) vs. other_chain
				bool collide = false;
				if (other_part->mType == FdNode::PATCH)
				{
					PatchNode* other_patch = (PatchNode*) other_part;
					if (fVolume.intersects(other_patch->mPatch))
						collide = true;
				}
				else
				{
					RodNode* other_rod = (RodNode*) other_rod;
					if (fVolume.intersects(other_rod->mRod))
						collide = true;
				}

				// add collision link
				if (collide)
				{
					ChainNode* other_cn = dy_graph->getChainNode(other_chain->mID);
					dy_graph->addCollisionLink(fn, other_cn);
				}
			}
		}
	}

	// debug: output dependency graph
	QString filePath = path + "/" + mID;
	dy_graph->saveAsImage(filePath);
}

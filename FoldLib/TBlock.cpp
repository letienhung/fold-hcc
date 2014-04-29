#include "TBlock.h"
#include "TChain.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "Numeric.h"
#include <QDir>


TBlock::TBlock( QVector<FdNode*> parts, PatchNode* panel, QString id, Geom::Box &bBox )
	:BlockGraph(parts, id)
{
	// type
	mType = BlockGraph::T_BLOCK;

	// panel
	mPanel = (PatchNode*)getNode(panel->mID);
	mPanel->properties["isCtrlPanel"] = true;

	// create chains
	double thr = mPanel->mBox.getExtent(mPanel->mPatch.Normal) * 2;
	foreach (FdNode* n, getFdNodes())
	{
		if (n->properties.contains("isCtrlPanel")) continue;

		if (getDistance(n, mPanel) < thr)
		{
			chains.push_back(new TChain(n, mPanel));
		}
	}
}

TBlock::~TBlock()
{
}

void TBlock::foldabilize()
{
	buildDependGraph();
}


void TBlock::buildDependGraph()
{
	//// clear
	//fog->clear();

	//// empty pizzaLayer
	//if (nodes.size() == 1) return;

	//// chain nodes and folding links
	//for(int i = 0; i < chains.size(); i++)
	//{
	//	TChain* chain = (TChain*)chains[i];

	//	// chain nodes
	//	ChainNode* cn = new ChainNode(i, chain->mID);
	//	fog->addNode(cn);

	//	for (int j = 0; j < chain->rootJointSegs.size(); j++)
	//	{
	//		//// folding nodes
	//		//QString fnid = chain->mID + "_" + QString::number(j);
	//		//QString fnid1 = fnid + "_" + QString::number(false);
	//		//FoldingNode* fn1 = new FoldingNode(j, false, fnid1);
	//		//Geom::SectorCylinder fVolume1 = chain->getFoldingVolume(fn1);
	//		//fn1->properties["fVolume"].setValue(fVolume1);
	//		//fog->addNode(fn1);

	//		//QString fnid2 = fnid + "_" + QString::number(true);
	//		//FoldingNode* fn2 = new FoldingNode(j, true, fnid2);
	//		//Geom::SectorCylinder fVolume2 = chain->getFoldingVolume(fn2);
	//		//fn2->properties["fVolume"].setValue(fVolume2);
	//		//fog->addNode(fn2);

	//		//// folding links
	//		//fog->addFoldingLink(cn, fn1);
	//		//fog->addFoldingLink(cn, fn2);
	//	}
	//}

	//// barrier nodes
	//QVector<Geom::Rectangle> barriers = barrierBox.getFaceRectangles();
	//Vector3 pNormal = mPanel->mPatch.Normal;
	//for (int i = 0; i < Geom::Box::NB_FACES; i++)
	//{
	//	double dotProd = fabs(dot(pNormal, barriers[i].Normal));
	//	if (dotProd > 0.5)  continue;
	//	fog->addNode(new BarrierNode(i));
	//}

	//// dependency links
	//foreach (FoldingNode* fn, fog->getAllFoldingNodes())
	//{
	//	ChainNode* cn = fog->getChainNode(fn->mID);
	//	TChain* chain = (TChain*) getChain(cn->mID);
	//	Geom::SectorCylinder fVolume = chain->getFoldingVolume(fn);

	//	// with barriers 
	//	foreach (BarrierNode* bn, fog->getAllBarrierNodes())
	//	{
	//		Geom::Rectangle& barrier = barriers[bn->faceIdx];
	//		if (fVolume.intersects(barrier))
	//		{
	//			fog->addCollisionLink(fn, bn);
	//		}
	//	}

	//	// with other chain nodes
	//	foreach(ChainNode* other_cn, fog->getAllChainNodes())
	//	{
	//		if (cn == other_cn) continue;

	//		TChain* other_chain = (TChain*) getChain(other_cn->mID);
	//		FdNode* other_part = other_chain->mParts[0];

	//		bool collide = false;
	//		if (other_part->mType == FdNode::PATCH)
	//		{
	//			PatchNode* other_patch = (PatchNode*) other_part;
	//			if (fVolume.intersects(other_patch->mPatch))
	//			{
	//				collide = true;
	//				//debugSegs << Geom::Segment(fVCenter, other_patch->mPatch.Center);
	//			}
	//		}else
	//		{
	//			RodNode* other_rod = (RodNode*) other_part;
	//			if (fVolume.intersects(other_rod->mRod))
	//			{
	//				collide = true;
	//				//debugSegs << Geom::Segment(fVCenter, other_rod->mRod.Center);
	//			}
	//		}

	//		// add collision link
	//		if (collide)
	//		{
	//			fog->addCollisionLink(fn, other_cn);
	//		}
	//	}
	//}
}

QVector<Structure::Node*> TBlock::getKeyFrameNodes( double t )
{
	QVector<Structure::Node*> knodes;

	// evenly distribute time among pizza chains
	QVector<double> chainStarts = getEvenDivision(chains.size());

	// chain parts
	// fold in sequence
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

Vector3 TBlock::getClosestCoordinates(Geom::SectorCylinder& fVolume, FdNode* node)
{
	Geom::Segment axisSeg = fVolume.getAxisSegment();
	Vector3 closestP;
	if (node->mType == FdNode::PATCH)
	{
		PatchNode* pnode = (PatchNode*) node;
		Geom::DistSegRect dsr(axisSeg, pnode->mPatch);
		closestP = dsr.mClosestPoint1;
	}
	else
	{
		RodNode* rnode = (RodNode*) node;
		Geom::DistSegSeg dss(axisSeg, rnode->mRod);
		closestP = dss.mClosestPoint1;
	}

	return fVolume.getCoordinates(closestP);
}

Vector3 TBlock::getClosestCoordinates( Geom::SectorCylinder& fVolume, Geom::Rectangle& rect )
{
	Geom::Segment axisSeg = fVolume.getAxisSegment();
	Geom::DistSegRect dsr(axisSeg, rect);

	Vector3 closestP = dsr.mClosestPoint1;

	return fVolume.getCoordinates(closestP);
}
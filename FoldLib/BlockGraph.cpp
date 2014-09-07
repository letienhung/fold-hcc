#include "BlockGraph.h"
#include "FdUtility.h"
#include "Numeric.h"
#include "ChainGraph.h"
#include "TChainGraph.h"

BlockGraph::BlockGraph(QString id, Geom::Box shape_aabb)
	: FdGraph(id)
{
	// selected chain
	selChainIdx = -1;

	// shape AABB
	shapeAABB = shape_aabb;

	// parameters for split and shrink
	nbSplits = 1;
	nbChunks = 2;

	// thickness
	thickness = 2;
	useThickness = false;

	// selected solution
	selSlnIdx = -1;

	// cost weight
	weight = 0.05;

	// foldabilized tag
	foldabilized = false;
}

BlockGraph::~BlockGraph()
{
	foreach(ChainGraph* c, chains)
		delete c;
}

FdGraph* BlockGraph::activeScaffold()
{
	FdGraph* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

ChainGraph* BlockGraph::getSelChain()
{
	if (selChainIdx >= 0 && selChainIdx < chains.size())
		return chains[selChainIdx];
	else
		return NULL;
}

void BlockGraph::selectChain( QString id )
{
	selChainIdx = -1;
	for (int i = 0; i < chains.size(); i++)
	{
		if (chains[i]->mID == id)
		{
			selChainIdx = i;
			break;
		}
	}
}

QStringList BlockGraph::getChainLabels()
{
	QStringList labels;
	foreach(FdGraph* c, chains)
		labels.push_back(c->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

// The super keyframe is the keyframe + superPatch
// which is an additional patch representing the folded block
FdGraph* BlockGraph::getSuperKeyframe( double t )
{
	// regular key frame w\o thickness
	FdGraph* keyframe = getKeyframe(t, false);

	// do nothing if the block is NOT fully folded
	if (1 - t > ZERO_TOLERANCE_LOW) return keyframe;

	// create super patch
	PatchNode* superPatch = (PatchNode*)baseMaster->clone();
	superPatch->mID = mID + "_super";
	superPatch->addTag(SUPER_PATCH_TAG); 

	// collect projections of all nodes on baseMaster
	Geom::Rectangle base_rect = superPatch->mPatch;
	QVector<Vector2> projPnts2 = base_rect.get2DConners();
	if (foldabilized)
	{// all nodes since they are folded flatly already
		foreach (FdNode* n, keyframe->getFdNodes())
		{
			if (n->mType == FdNode::PATCH)
			{
				Geom::Rectangle part_rect = ((PatchNode*)n)->mPatch;
				projPnts2 << base_rect.get2DRectangle(part_rect).getConners();
			}
			else
			{
				Geom::Segment part_rod = ((RodNode*)n)->mRod;
				projPnts2 << base_rect.getProjCoordinates(part_rod.P0);
				projPnts2 << base_rect.getProjCoordinates(part_rod.P1);
			}
		}
	}else
	{// guess the folded state using available folding regions
		for(auto afr : availFoldingRegion)
			projPnts2 << afr.getConners();
	}

	// resize super patch
	Geom::Rectangle2 aabb2 = computeAABB2D(projPnts2);
	superPatch->resize(aabb2);

	// merged parts
	foreach (FdNode* n, keyframe->getFdNodes())
	{
		n->addTag(MERGED_PART_TAG); 
		if (n->mType == FdNode::PATCH)
			superPatch->appendToSetProperty<QString>(MERGED_MASTERS, n->mID);
	}

	// add super patch to keyframe
	keyframe->Structure::Graph::addNode(superPatch);

	return keyframe;
}

double BlockGraph::getNbTopMasters()
{
	return nbMasters(this) - 1;
}

void BlockGraph::setThickness( double thk )
{
	thickness = thk;
	foreach (ChainGraph* chain, chains)	
	{
		chain->halfThk = thickness / 2;
		chain->baseOffset = thickness / 2;
	}
}

void BlockGraph::exportCollFOG()
{
	// do nothing
}

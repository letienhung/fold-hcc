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
	maxNbSplits = 1;
	maxNbChunks = 2;

	// thickness
	thickness = 2;
	useThickness = false;

	// cost weight
	weight = 0.05;
	
	// tag
	foldabilized = false;

	// current fold solution
	currSlnIdx = -1;
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
	if (t < 1) return keyframe;

	// create super patch
	PatchNode* superPatch = (PatchNode*)baseMaster->clone();
	superPatch->mID = mID + "_super";
	superPatch->addTag(SUPER_MASTER_TAG); 

	// collect projections of all nodes (including baseMaster) on baseMaster
	Geom::Rectangle base_rect = superPatch->mPatch;
	QVector<Vector2> projPnts2 = base_rect.get2DConners();
	foreach(FdNode* n, keyframe->getFdNodes())
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

	// resize super patch
	Geom::Rectangle2 aabb2 = Geom::Rectangle2::computeAABB(projPnts2);
	superPatch->resize(aabb2);

	// merged parts and masters
	foreach (FdNode* n, keyframe->getFdNodes())
	{
		n->addTag(MERGED_PART_TAG); 
		if (n->hasTag(MASTER_TAG))
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

QVector<QString> BlockGraph::getInbetweenExternalParts(Vector3 base_center, Vector3 top_center, ShapeSuperKeyframe* ssKeyframe)
{
	// time line
	Vector3 sqzV = baseMaster->mPatch.Normal;
	Geom::Line timeLine(Vector3(0, 0, 0), sqzV);

	// position on time line
	double t0 = timeLine.getProjTime(base_center);
	double t1 = timeLine.getProjTime(top_center);
	double epsilon = 0.05 * (t1 - t0);
	Interval m1m2 = INTERVAL(t0 + epsilon, t1 - epsilon);

	// find parts in between m1 and m2
	QVector<QString> inbetweens;
	foreach(FdNode* n, ssKeyframe->getFdNodes())
	{
		// skip parts that has been folded
		if (n->hasTag(MERGED_PART_TAG)) continue;

		// skip parts in this block
		if (containsNode(n->mID)) continue;

		// master
		if (n->hasTag(MASTER_TAG))
		{
			double t = timeLine.getProjTime(n->center());

			if (within(t, m1m2)) 	inbetweens << n->mID;
		}
		else
			// slave		
		{
			int aid = n->mBox.getAxisId(sqzV);
			Geom::Segment sklt = n->mBox.getSkeleton(aid);
			double t0 = timeLine.getProjTime(sklt.P0);
			double t1 = timeLine.getProjTime(sklt.P1);
			if (t0 > t1) std::swap(t0, t1);
			Interval ti = INTERVAL(t0, t1);

			if (overlap(ti, m1m2))	inbetweens << n->mID;
		}
	}

	return inbetweens;
}

void BlockGraph::genAllFoldOptions()
{
	// clear
	allFoldOptions.clear();

	// collect all
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (int i = 0; i < chains.size(); i++){
		for (auto fo : chains[i]->genFoldOptions(maxNbSplits, maxNbChunks))
		{
			// chain index
			fo->chainIdx = i;

			// region projection
			fo->regionProj = base_rect.get2DRectangle(fo->region);

			// store
			allFoldOptions << fo;

			// index
			fo->index = allFoldOptions.size() - 1;
		}
	}
}

int BlockGraph::searchForExistedSolution(const QVector<int>& afo)
{
	for (int i = 0; i < testedAvailFoldOptions.size(); i++)
		if (testedAvailFoldOptions[i] == afo) return i;

	return -1;
}

double BlockGraph::foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe)
{
	QVector<int> afo = getAvailFoldOptions(ssKeyframe);
	
	// search for existed solutions
	currSlnIdx = searchForExistedSolution(afo);
	if (currSlnIdx >= 0 && currSlnIdx < testedAvailFoldOptions.size())
		return foldCost[currSlnIdx];

	// find the optimal solution
	double cost = findOptimalSolution(afo);

	// return the cost (\in [0, 1])
	return cost;
}

void BlockGraph::applySolution()
{
	// assert idx
	if (currSlnIdx < 0 || currSlnIdx >= foldSolutions.size())
	{
		foldabilized = false;
		return;
	}

	// apply selected fold option to each chain
	for (int i = 0; i < chains.size(); i++)
	{
		FoldOption* fn = foldSolutions[currSlnIdx][i];
		chains[i]->applyFoldOption(fn);
	}

	// has been foldabilized
	foldabilized = true;
}

double BlockGraph::computeCost(FoldOption* fo)
{
	// split
	double a = fo->nSplits / (double)maxNbSplits;

	// shrinking
	double b = fo->scale;

	// blended cost : c \in [0, 1]
	double c = weight * a + (1 - weight) * b;

	// normalized
	double cost = chainWeights[fo->chainIdx] * c;

	// return
	return cost;
}

double BlockGraph::getChainArea()
{
	double a = 0;
	for (auto c : chains)a += c->getArea();
	return a;
}

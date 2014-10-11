#include "UnitScaff.h"
#include "FdUtility.h"
#include "Numeric.h"
#include "ChainScaff.h"
#include "TChainScaff.h"
#include "GeomUtility.h"

UnitScaff::UnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >&) : Scaffold(id)
{
	// clone nodes
	for (PatchNode* m : ms)	{
		masters << (PatchNode*)m->clone();
		Structure::Graph::addNode(masters.last());
	}
	for (ScaffNode* s : ss)
		Structure::Graph::addNode(s->clone());

	// selected chain index
	selChainIdx = -1;

	// upper bound of modification
	maxNbSplits = 1;
	maxNbChunks = 2;

	// cost weight
	weight = 0.05;
	importance = 0;  // importance 
	
	// current fold solution
	currSlnIdx = -1;

	// thickness
	thickness = 2;
	useThickness = false;
}

UnitScaff::~UnitScaff()
{
	for (ChainScaff* c : chains)
		delete c;

	for (UnitSolution* sln : testedSlns)
		delete sln;
}

void UnitScaff::setAabbCstr(Geom::Box aabb)
{
	aabbCstr = aabb;

	Geom::Rectangle base_rect = baseMaster->mPatch;
	int aid = aabbCstr.getAxisId(base_rect.Normal);
	Geom::Rectangle cs_rect = aabbCstr.getCrossSection(aid, 0);
	aabbCstrProj = base_rect.get2DRectangle(cs_rect);
}

void UnitScaff::setNbSplits(int n)
{
	maxNbSplits = n;
}

void UnitScaff::setNbChunks(int n)
{
	maxNbChunks = n;
}

void UnitScaff::setCostWeight(double w)
{
	weight = w;
}

void UnitScaff::setThickness(double thk)
{
	thickness = thk;
	for (ChainScaff* chain : chains)
	{
		chain->halfThk = thickness / 2;
		chain->baseOffset = thickness / 2;
	}
}


Scaffold* UnitScaff::activeScaffold()
{
	Scaffold* selChain = getSelChain();
	if (selChain)  return selChain;
	else		   return this;
}

ChainScaff* UnitScaff::getSelChain()
{
	if (selChainIdx >= 0 && selChainIdx < chains.size())
		return chains[selChainIdx];
	else
		return nullptr;
}

void UnitScaff::selectChain( QString id )
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

QStringList UnitScaff::getChainLabels()
{
	QStringList labels;
	for (Scaffold* c : chains)
		labels.push_back(c->mID);

	// append string to select none
	labels << "--none--";

	return labels;
}

// The super keyframe is the keyframe + superPatch
// which is an additional patch representing the folded block
Scaffold* UnitScaff::getSuperKeyframe( double t )
{
	// regular key frame w\o thickness
	Scaffold* keyframe = getKeyframe(t, false);

	// do nothing if the block is NOT fully folded
	if (t < 1) return keyframe;

	// create super patch
	PatchNode* superPatch = (PatchNode*)baseMaster->clone();
	superPatch->mID = mID + "_super";
	superPatch->addTag(SUPER_MASTER_TAG); 

	// collect projections of all nodes (including baseMaster) on baseMaster
	Geom::Rectangle base_rect = superPatch->mPatch;
	QVector<Vector2> projPnts2 = base_rect.get2DConners();
	for (ScaffNode* n : keyframe->getScaffNodes())
	{
		if (n->mType == ScaffNode::PATCH)
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
	Geom::Rectangle2 aabb2 = Geom::computeAABB(projPnts2);
	superPatch->resize(aabb2);

	// merged parts and masters
	for(Structure::Node* n : keyframe->nodes)
	{
		n->addTag(MERGED_PART_TAG); 
		if (n->hasTag(MASTER_TAG))
			superPatch->appendToSetProperty<QString>(MERGED_MASTERS, n->mID);
	}

	// add super patch to keyframe
	keyframe->Structure::Graph::addNode(superPatch);

	return keyframe;
}

double UnitScaff::getNbTopMasters()
{
	return getNodesWithTag(MASTER_TAG).size() - 1;
}

void UnitScaff::genAllFoldOptions()
{
	// clear
	allFoldOptions.clear();

	// collect all
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (int i = 0; i < chains.size(); i++)
	{
		// regular option
		for (FoldOption* fo : chains[i]->genRegularFoldOptions(maxNbSplits, maxNbChunks))
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

		// delete option
		FoldOption* dfo = chains[i]->genDeleteFoldOption(maxNbSplits);
		dfo->chainIdx = i;
		allFoldOptions << dfo;
		dfo->index = allFoldOptions.size() - 1;
	}
}


double UnitScaff::foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti)
{
	// time interval
	mFoldDuration = ti;

	// create a new solution
	UnitSolution* fdSln = new UnitSolution();

	// available fold options
	computeAvailFoldOptions(ssKeyframe, fdSln);
	
	// search for existed solutions
	currSlnIdx = -1;
	for (int i = 0; i < testedSlns.size(); i++){
		if (testedSlns[i]->afo == fdSln->afo)
		{
			delete fdSln;
			currSlnIdx = i;
			return testedSlns[currSlnIdx]->cost;
		}
	}

	// find new solution
	findOptimalSolution(fdSln);

	// store the solution
	currSlnIdx = testedSlns.size();
	testedSlns << fdSln;

	// apply the solution
	for (int i = 0; i < chains.size(); i++)
		chains[i]->applyFoldOption(testedSlns[currSlnIdx]->solution[i]);

	// return the cost (\in [0, 1])
	return fdSln->cost;
}

double UnitScaff::computeCost(FoldOption* fo)
{
	// split
	double a = fo->nSplits / (double)maxNbSplits;

	// shrinking
	double b = 1 - fo->scale;

	// blended cost : c \in [0, 1]
	double c = weight * a + (1 - weight) * b;

	// normalized
	double cost = chains[fo->chainIdx]->importance * c;

	// return
	return cost;
}

double UnitScaff::getTotalSlaveArea()
{
	double a = 0;
	for (ChainScaff* c : chains)a += c->getSlaveArea();
	return a;
}


void UnitScaff::resetAllFoldOptions()
{
	// regenerate all fold options
	genAllFoldOptions();

	// clear saved solutions
	for (UnitSolution* sln : testedSlns) 
		delete sln;
	testedSlns.clear();
	currSlnIdx = -1;
}

void UnitScaff::computeChainImportances()
{
	double totalA = 0;
	for (ChainScaff* c : chains)
	{
		double area = c->getSlaveArea();
		totalA += area;
	}

	for (ChainScaff* c : chains)
		c->importance = c->getSlaveArea() / totalA;
}

void UnitScaff::computeAvailFoldOptions(SuperShapeKf*, UnitSolution*)
{
}

void UnitScaff::findOptimalSolution(UnitSolution*)
{
}

void UnitScaff::setImportance(double imp)
{
	importance = imp;
}

QVector<Vector3> UnitScaff::getObstacles()
{
	QVector<Vector3> obs;
	if (currSlnIdx >= 0 && currSlnIdx < testedSlns.size())
		obs = testedSlns[currSlnIdx]->obstacles;

	return obs;
}

QVector<Geom::Rectangle> UnitScaff::getAFRs()
{
	QVector<Geom::Rectangle> afr;
	if (currSlnIdx >= 0 && currSlnIdx < testedSlns.size())
	{
		for (int foi : testedSlns[currSlnIdx]->afo)
			afr << allFoldOptions[foi]->region;
	}

	return afr;
}

void UnitScaff::genDebugInfo()
{
	properties.clear();

	// obstacles
	appendToVectorProperty(DEBUG_POINTS, getObstacles());
	
	// available fold options/regions
	for (Geom::Rectangle rect : getAFRs())
	{
		appendToVectorProperty(DEBUG_SEGS, rect.getEdgeSegments());
		//appendToVectorProperty(DEBUG_POINTS, rect.getConners());
	}

	// aabb constraint
	appendToVectorProperty(DEBUG_SEGS, baseMaster->mPatch.get3DRectangle(aabbCstrProj).getEdgeSegments());
}


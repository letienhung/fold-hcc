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
	
	// thickness
	thickness = 2;
	useThickness = false;
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

UnitScaff::~UnitScaff()
{
	for (ChainScaff* c : chains)
		delete c;
}


void UnitScaff::setAabbCstr(Geom::Box aabb)
{
	aabbCstr = aabb;
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

void UnitScaff::setImportance(double imp)
{
	importance = imp;
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


double UnitScaff::getNbTopMasters()
{
	return getNodesWithTag(MASTER_TAG).size() - 1;
}

double UnitScaff::getTotalSlaveArea()
{
	double a = 0;
	for (ChainScaff* c : chains)a += c->getSlaveArea();
	return a;
}


void UnitScaff::genDebugInfo()
{
	visDebug.clearAll();

	// obstacles
	visDebug.addPoints(getCurrObstacles(), Qt::blue);

	// available fold options/regions
	visDebug.addRectangles(getCurrAFRs(), Qt::green);
}


// The super keyframe is the keyframe + superPatch
// which is an additional patch representing the folded block
Scaffold* UnitScaff::getSuperKeyframe(double t)
{
	// regular key frame w\o thickness
	Scaffold* keyframe = getKeyframe(t, false);

	// do nothing if the block is NOT fully folded
	if (t < 1) return keyframe;

	// create super patch
	SuperPatchNode* superPatch = new SuperPatchNode(mID + "_super", baseMaster);
	superPatch->addTag(SUPER_PATCH_TAG);

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
	for (Structure::Node* n : keyframe->nodes)
	{
		n->addTag(MERGED_PART_TAG);
		if (n->hasTag(MASTER_TAG))	superPatch->enclosedPatches << n->mID;
	}

	// add super patch to keyframe
	keyframe->Structure::Graph::addNode(superPatch);

	return keyframe;
}


bool UnitScaff::isExternalPart(ScaffNode* snode)
{
	bool isInternal;
	if (snode->hasTag(SUPER_PATCH_TAG))
	{
		isInternal = false;
		for (QString mm : ((SuperPatchNode*)snode)->enclosedPatches){
			if (containsNode(mm))
			{
				isInternal = true;
				break;
			}
		}
	}
	// regular part
	else
	{
		isInternal = containsNode(snode->mID);
	}

	return !isInternal;
}

QVector<Vector3> UnitScaff::computeObstaclePnts(SuperShapeKf* ssKeyframe, QString base_mid, QString top_mid)
{
	// obstacle parts
	QVector<ScaffNode*> candidateParts, obstParts;
	candidateParts << ssKeyframe->getInbetweenParts(base_mid, top_mid);
	candidateParts << ssKeyframe->getUnrelatedMasters(base_mid);
	candidateParts << ssKeyframe->getUnrelatedMasters(top_mid);
	for (ScaffNode* sn : candidateParts) if (isExternalPart(sn)) obstParts << sn;

	// sample obstacle parts
	QVector<Vector3> obstPnts;
	for (ScaffNode* obsPart : obstParts)
		obstPnts << obsPart->sampleBoundabyOfScaffold(100);

	return obstPnts;
}

Geom::Rectangle2 UnitScaff::getAabbCstrProj(Geom::Rectangle& base_rect)
{
	int aid = aabbCstr.getAxisId(base_rect.Normal);
	Geom::Rectangle aabbCrossSect = aabbCstr.getCrossSection(aid, 0);

	return base_rect.get2DRectangle(aabbCrossSect);
}

Geom::Rectangle UnitScaff::getBaseRect(SuperShapeKf* ssKeyframe)
{
	return ((PatchNode*)ssKeyframe->getNode(baseMaster->mID))->mPatch;
}

QVector<QString> UnitScaff::getSlnSlaveParts()
{
	QVector<QString> sParts;
	for (ChainScaff* chain : chains)
	{
		for (PatchNode* cs : chain->chainParts)
		{
			sParts << cs->mID;
		}
	}

	return sParts;
}

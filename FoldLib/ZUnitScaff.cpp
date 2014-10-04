#include "ZUnitScaff.h"
#include "TChainScaff.h"

ZUnitScaff::ZUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) : UnitScaff(id, ms, ss, mPairs)
{
	// create the back up HUnit
	hUnit = new HUnitScaff("H_"+id, ms, ss, mPairs);

	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();

	// two possible fold solution
	// the right direction is the rightSegV of first chain
	Vector3 rightV = chains.front()->rightSegV;
	computeFoldSolution(rightV);
	computeFoldSolution(-rightV);
	computeFoldRegionProj(true);
	computeFoldRegionProj(false);
	fold2Left = false;
	fold2Right = false;
}

void ZUnitScaff::sortMasters()
{
	// the base and top master
	QMap<QString, double> masterTimeStamps =
		getTimeStampsNormalized(masters, masters.front()->mPatch.Normal, timeScale);
	if (masterTimeStamps[masters.front()->mID] < ZERO_TOLERANCE_LOW)
	{
		baseMaster = masters.front();	
		topMaster = masters.last();
	}
	else
	{
		baseMaster = masters.last();	
		topMaster = masters.front();
	}
}

void ZUnitScaff::createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >&)
{
	for (ScaffNode* slave : ss)
	{
		ChainScaff* tc = new TChainScaff(slave, baseMaster, topMaster);
		tc->setFoldDuration(0, 1);
		chains << tc;
	}
}


void ZUnitScaff::computeFoldSolution(Vector3 rV)
{
	// 
}


void ZUnitScaff::computeFoldRegionProj(bool isRight)
{
	// 
}


void ZUnitScaff::setImportance(double imp)
{
	// z unit
	UnitScaff::setImportance(imp);

	// h unit
	hUnit->setImportance(imp);
}


QVector<Vector2> ZUnitScaff::computeObstacles(ShapeSuperKeyframe* ssKeyframe)
{
	// in-between external parts
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QVector<ScaffNode*> obstacleParts = ssKeyframe->getInbetweenExternalParts(this, baseMaster->center(), topMaster->center());

	// sample obstacle parts
	QVector<Vector3> samples;
	for (ScaffNode* obsPart : obstacleParts)
		samples << obsPart->sampleBoundabyOfScaffold(100);

	// projection
	obstacles.clear();
	for (Vector3 s : samples)
		obstacles << base_rect.getProjCoordinates(s);

	return obstacles;
}




bool ZUnitScaff::foldabilizeAsZ(ShapeSuperKeyframe* ssKeyframe)
{
	// obstacles
	QVector<Vector2> obsPnts = computeObstacles(ssKeyframe);

	// feasibility of left solution
	bool isCollidingLeft = leftRegionProj.containsAny(obstacles, -0.01);
	bool inAABBLeft = aabbConstraint.containsAll(leftRegionProj.getConners(), 0.01);
	fold2Left = !isCollidingLeft && inAABBLeft;

	// feasibility of right solution
	bool isCollidingRight = rightRegionProj.containsAny(obstacles, -0.01);
	bool inAABBRight = aabbConstraint.containsAll(rightRegionProj.getConners(), 0.01);
	fold2Right = !isCollidingRight && inAABBRight;

	// success if one side works
	return fold2Left || fold2Right;
}


double ZUnitScaff::foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe)
{
	// Z-folding
	double cost = 0;
	bool zSuccess = foldabilizeAsZ(ssKeyframe);

	// fold as H
	if (!zSuccess)
	{
		cost = hUnit->foldabilizeWrt(ssKeyframe);
	}

	return cost;
}


Scaffold* ZUnitScaff::getKeyframeAsZ(double t, bool useThk)
{
	// the unit is not ready to fold
	if (t <= 0)	return (Scaffold*)this->clone();

	// masters
	Scaffold* keyframe = nullptr;
	keyframe->Structure::Graph::addNode(baseMaster->clone());

	// slaves
	for (int i = 0; i < chains.size(); i++)
	{
		double localT = chains[i]->duration.getLocalTime(t);
		Scaffold* ck = chains[i]->getKeyframe(localT, useThk);
	}


	// the key frame
	return keyframe;
}


Scaffold* ZUnitScaff::getKeyframe(double t, bool useThk)
{
	if (fold2Left || fold2Right)
		return getKeyframeAsZ(t, useThk);
	else
		return hUnit->getKeyframe(t, useThk);
}

void ZUnitScaff::applySolution()
{
	if (fold2Left || fold2Right)
	{
		// apply Z solution : choose any
		QVector<FoldOption*>& options = fold2Left ? leftOptions : rightOptions;
		for (int i = 0; i < chains.size(); i++)
		{
			chains[i]->applyFoldOption(options[i]);
		}
	}
	else
	{
		// apply H solution
		hUnit->applySolution();
	}
}
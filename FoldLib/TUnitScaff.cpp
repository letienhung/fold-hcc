#include "TUnitScaff.h"
#include "TChainScaff.h"
#include "Numeric.h"

TUnitScaff::TUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) :UnitScaff(id, ms, ss, mPairs)
{
	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();

	// generate all fold options
	genAllFoldOptions();
}


void TUnitScaff::sortMasters()
{
	// the base and virtual top masters
	bool vFront = masters.front()->hasTag(EDGE_ROD_TAG);
	topMaster = vFront ? masters.front() : masters.last();
	baseMaster = vFront ? masters.last() : masters.front();
}

void TUnitScaff::createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& )
{
	// in case that the T chain is up-side-down
	// make sure the normal of base master pointing to the same side as the chain part
	Vector3 chain2base = (ss.front()->center() - baseMaster->center()).normalized();
	if (dot(baseMaster->mPatch.Normal, chain2base) < 0)
		baseMaster->mPatch.flipNormal();

	// create the chain
	tChain = new TChainScaff(ss.front(), baseMaster, topMaster);
	tChain->setFoldDuration(0, 1);

	// store to chains
	chains << tChain;
}


// the keyframe cannot be nullptr
Scaffold* TUnitScaff::getKeyframe(double t, bool useThk)
{
	return tChain->getKeyframe(t, useThk);
}

void TUnitScaff::computeObstacles(SuperShapeKf* ssKeyframe, UnitSolution* sln)
{
	// obstacle points
	QVector<Vector3> obstPnts = computeObstaclePnts(ssKeyframe, baseMaster->mID, topMaster->mID);

	// projection
	obstacles.clear();
	for (Vector3 s : obstPnts)
		obstacles << sln->baseRect.getProjCoordinates(s);

	// store obstacles in solution
	sln->obstacles = obstPnts;
	sln->obstaclesProj.clear();
	for (Vector3 s : obstPnts)
		sln->obstaclesProj << sln->baseRect.getProjection(s);
}


void TUnitScaff::computeAvailFoldOptions(SuperShapeKf* ssKeyframe, UnitSolution* sln)
{
	computeObstacles(ssKeyframe, sln);

	// prune fold options
	sln->afoIndices.clear();
	for (int i = 0; i < allFoldOptions.size(); i++)
	{
		// the fold option
		FoldOption* fo = allFoldOptions[i];

		// always accept the delete fold option
		// check acceptance for regular fold option
		bool accepted = true;
		if (fo->scale != 0)
		{
			// prune
			bool isColliding = fo->regionProj.containsAny(obstacles, -0.01);
			bool inAABB = sln->aabbCstrProj.containsAll(fo->regionProj.getConners(), 0.01);
			accepted = !isColliding && inAABB;
		}
		
		// store
		if (accepted) sln->afoIndices << i;
	}
}

void TUnitScaff::findOptimalSolution(UnitSolution* sln)
{
	// choose the one with the lowest cost
	int bestIdx = -1;
	sln->cost = maxDouble();
	for(int fi : sln->afoIndices)
	{
		FoldOption* fo = allFoldOptions[fi];
		double cost = computeCost(fo);
		if (cost < sln->cost)
		{
			bestIdx = fi;
			sln->cost = cost;
		}
	}

	sln->chainSln.clear();
	sln->chainSln << bestIdx;
}

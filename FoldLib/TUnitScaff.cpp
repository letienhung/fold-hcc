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

QVector<Vector2> TUnitScaff::computeObstacles(SuperShapeKf* ssKeyframe)
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


QVector<int> TUnitScaff::getAvailFoldOptions(SuperShapeKf* ssKeyframe)
{
	QVector<Vector2> obstacles = computeObstacles(ssKeyframe);

	// prune fold options
	QVector<int> afo;
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
			bool inAABB = aabbConstraint.containsAll(fo->regionProj.getConners(), 0.01);
			accepted = !isColliding && inAABB;
		}
		
		// store
		if (accepted) afo << i;
	}

	// result
	return afo;
}

double TUnitScaff::findOptimalSolution(const QVector<int>& afo)
{
	// choose the one with the lowest cost
	FoldOption* best_fo = nullptr;
	double minCost = maxDouble();
	for(auto fi : afo){
		FoldOption* fo = allFoldOptions[fi];
		double cost = computeCost(fo);
		if (cost < minCost)
		{
			best_fo = fo;
			minCost = cost;
		}
	}

	// store the solution
	testedAvailFoldOptions << afo;
	foldSolutions << (QVector<FoldOption*>() << best_fo);
	foldCost << minCost;

	// update the current solution
	currSlnIdx = foldSolutions.size() - 1;

	// return the cost
	return minCost;
}

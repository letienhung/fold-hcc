#include "TUnitScaff.h"
#include "TChainScaff.h"
#include "Numeric.h"

TUnitScaff::TUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& ) :UnitScaff(id)
{
	// clone nodes
	for(PatchNode* m : ms)	{
		masters << (PatchNode*)m->clone();
		Structure::Graph::addNode(masters.last());
	}
	for(ScaffNode* s : ss)
		Structure::Graph::addNode(s->clone());

	// the base and virtual top masters
	bool isVirtualFirst = masters.front()->hasTag(EDGE_ROD_TAG);
	topMaster = isVirtualFirst? masters.front() : masters.last();
	baseMaster = isVirtualFirst ? masters.last() : masters.front();

	// make sure the normal of base master pointing to the same side as the chain part
	Vector3 chain2base = (ss.front()->center() - baseMaster->center()).normalized();
	if (dot(baseMaster->mPatch.Normal, chain2base) < 0)
		baseMaster->mPatch.flipNormal();

	// the chain
	tChain = new TChainScaff(ss.front(), baseMaster, topMaster);
	tChain->setFoldDuration(0, 1);
	chains << tChain;

	// the chain weights
	chainWeights.clear();
	chainWeights << 1.0;
}

Scaffold* TUnitScaff::getKeyframe(double t, bool useThk)
{
	Scaffold* keyframe = nullptr;

	if (t <= 0)
		keyframe = (Scaffold*)this->clone();
	else
		keyframe = tChain->getKeyframe(t, useThk);

	return keyframe;
}

QVector<Vector2> TUnitScaff::computeObstacles(ShapeSuperKeyframe* ssKeyframe)
{
	// in-between external parts
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QVector<QString> obstacleParts = getInbetweenExternalParts(baseMaster->center(), topMaster->center(), ssKeyframe);

	// sample obstacle parts
	QVector<Vector3> samples;
	for (auto nid : obstacleParts)
		samples << ssKeyframe->getFdNode(nid)->sampleBoundabyOfScaffold(100);

	// projection
	obstacles.clear();
	for (auto s : samples)
		obstacles << base_rect.getProjCoordinates(s);

	return obstacles;
}


QVector<int> TUnitScaff::getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe)
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

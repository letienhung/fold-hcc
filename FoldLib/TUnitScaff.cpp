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

QVector<Vector2> TUnitScaff::computeObstacles(SuperShapeKf* ssKeyframe, UnitSolution* sln)
{
	// in-between external parts
	QVector<ScaffNode*> obstParts = ssKeyframe->getInbetweenExternalParts(this, baseMaster->center(), topMaster->center());

	// sample obstacle parts
	QVector<Vector3> obstPnts;
	for (ScaffNode* p : obstParts)
		obstPnts << p->sampleBoundabyOfScaffold(100);

	// projection
	QVector<Vector2> obstPntsProj;
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (Vector3 s : obstPnts)
		obstPntsProj << base_rect.getProjCoordinates(s);

	// store obstacles in solution
	sln->obstacles = obstPnts;
	sln->obstaclesProj.clear();
	for (Vector3 s : obstPnts)
		sln->obstaclesProj << base_rect.getProjection(s);

	// return 2D projection
	return obstPntsProj;
}


void TUnitScaff::computeAvailFoldOptions(SuperShapeKf* ssKeyframe, UnitSolution* sln)
{
	QVector<Vector2> obstProj = computeObstacles(ssKeyframe, sln);

	// prune fold options
	sln->afo.clear();
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
			bool isColliding = fo->regionProj.containsAny(obstProj, -0.01);
			bool inAABB = aabbCstrProj.containsAll(fo->regionProj.getConners(), 0.01);
			accepted = !isColliding && inAABB;
		}
		
		// store
		if (accepted) sln->afo << i;
	}
}

void TUnitScaff::findOptimalSolution(UnitSolution* sln)
{
	// choose the one with the lowest cost
	FoldOption* best_fo = nullptr;
	sln->cost = maxDouble();
	for(auto fi : sln->afo){
		FoldOption* fo = allFoldOptions[fi];
		double cost = computeCost(fo);
		if (cost < sln->cost)
		{
			best_fo = fo;
			sln->cost = cost;
		}
	}

	sln->solution.clear();
	sln->solution << best_fo;
}

#include "TUnitScaff.h"
#include "TChainScaff.h"
#include "Numeric.h"
#include "ParSingleton.h"

TUnitScaff::TUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) :UnitScaff(id, ms, ss, mPairs)
{
	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();
}


void TUnitScaff::sortMasters()
{
	// the base and virtual top masters
	bool vFront = masters.front()->hasTag(EDGE_VIRTUAL_TAG);
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


Scaffold* TUnitScaff::getKeyframe(double t, bool useThk)
{
	return tChain->getKeyframe(t, useThk);
}

double TUnitScaff::foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti)
{
	// time interval
	mFoldDuration = ti;

	// obstacles
	obstacles = computeObstaclePnts(ssKeyframe, baseMaster->mID, topMaster->mID);

	{// debug
		
		visDebug.clearAll();
		visDebug.addPoints(obstacles, Qt::blue);
	}

	// projected coordinates on the base rect
	baseRect = getBaseRect(ssKeyframe);
	QVector<Vector2> obstacleProj;
	for (Vector3 s : obstacles)
		obstacleProj << baseRect.getProjCoordinates(s);

	// projected aabb constraint on the base rect
	Geom::Rectangle2 aabbCstrProj = getAabbCstrProj(baseRect);

	{// debug
		
		//FoldOption* dfo = sortedFoldOptions.front();
		//Geom::Rectangle tmRect = baseRect.get3DRectangle(dfo->regionProj);
		//visDebug.addRectangle(tmRect, Qt::blue);
		//visDebug.addRectangle(dfo->region, Qt::blue);
		//Geom::Rectangle tmRect = baseRect.get3DRectangle(aabbCstrProj);
		//visDebug.addRectangle(tmRect, Qt::blue);
	}

	// search for the best available fold option
	for (FoldOption* fo : sortedFoldOptions)
	{
		// always accept the delete fold option
		// check acceptance for regular fold option
		bool accepted = true;
		if (fo->scale != 0)
		{
			// prune
			double thr = 0.05;
			bool isColliding = fo->regionProj.containsAny(obstacleProj, -thr);
			bool inAABB = aabbCstrProj.containsAll(fo->regionProj.getConners(), thr);
			accepted = !isColliding && inAABB;

			{// debug

				Geom::Rectangle tmRect = baseRect.get3DRectangle(fo->regionProj);
				if (accepted)
					visDebug.addRectangle(tmRect, Qt::green);
				else
					visDebug.addRectangle(tmRect, Qt::red);
			}

		}

		// store
		if (accepted)
		{
			solution = fo;
			break;
		}
	}

	// apply the solution
	tChain->applyFoldOption(solution);

	// return the cost
	return solution->cost;
}

void TUnitScaff::initFoldSolution()
{
	// generate all fold options
	sortedFoldOptions.clear();
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (FoldOption* fo : tChain->genRegularFoldOptions())
	{
		fo->regionProj = base_rect.get2DRectangle(fo->region);
		fo->computeCost(1);
		fo->index = sortedFoldOptions.size();
		sortedFoldOptions << fo;
	}
	FoldOption* dfo = tChain->genDeleteFoldOption();
	dfo->computeCost(1);
	dfo->index = sortedFoldOptions.size();
	sortedFoldOptions << dfo;

	// sort
	qSort(sortedFoldOptions.begin(), sortedFoldOptions.end(),
		[](FoldOption* fo1, FoldOption* fo2){return fo1->cost < fo2->cost; });

	// solution
	solution = nullptr;
}

QVector<Vector3> TUnitScaff::getCurrObstacles()
{
	return obstacles;
}

QVector<Geom::Rectangle> TUnitScaff::getCurrAFRs()
{
	return getCurrSlnFRs();
}

QVector<Geom::Rectangle> TUnitScaff::getCurrSlnFRs()
{
	Geom::Rectangle rect = baseRect.get3DRectangle(solution->regionProj);
	return QVector<Geom::Rectangle>() << rect;
}

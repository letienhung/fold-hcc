#include "ZUnitScaff.h"
#include "TChainScaff.h"
#include "GeomUtility.h"

ZUnitScaff::ZUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
	QVector< QVector<QString> >& mPairs) : UnitScaff(id, ms, ss, mPairs)
{
	// decompose
	sortMasters();
	createChains(ss, mPairs);
	computeChainImportances();

	// create the back up HUnit
	hUnit = new HUnitScaff("H_"+id, ms, ss, mPairs);
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

void ZUnitScaff::initFoldSolution()
{
	// two possible fold solution
	prepareFoldSolution(true);
	prepareFoldSolution(false);
	computeFoldRegionProj(true);
	computeFoldRegionProj(false);
	fold2Left = false;
	fold2Right = false;

	// initialize the backup H-Unit
	hUnit->initFoldSolution();
}


void ZUnitScaff::prepareFoldSolution(bool toRight)
{
	// the fold direction
	Vector3 foldV = chains.front()->rightDirect;
	if (!toRight) foldV *= -1;

	// the solution
	QVector<FoldOption*>& options = toRight ? optionsRight : optionsLeft;
	options.clear();
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (ChainScaff* chain : chains)
	{
		QString id = toRight ? chain->mID + "_R" : chain->mID + "_L";
		bool toChainRight = dot(foldV, chain->rightDirect) > 0;
		FoldOption* fo = chain->genFoldOption(id, toChainRight, 1, 0, 0);
		fo->regionProj = base_rect.get2DRectangle(fo->region);
		options << fo;
	}
}

void ZUnitScaff::computeFoldRegionProj(bool toRight)
{
	// folded slaves
	Geom::Rectangle base_rect = baseMaster->mPatch;
	QVector<Vector2> samples;	// samples on base_rect
	QVector<FoldOption*>& options = toRight ? optionsRight : optionsLeft;
	for (FoldOption* fo : options) 
		samples << fo->regionProj.getConners();

	// top master 
	Geom::Rectangle top_rect = topMaster->mPatch;
	samples << base_rect.get2DRectangle(top_rect).getConners();

	// top master at final position
	ChainScaff* chain = chains.front();
	Vector3 rightV = chain->slaveSeg.length() * chain->rightDirect;
	Vector3 slaveV = chain->slaveSeg.length() * chain->slaveSeg.Direction;
	Vector3 topTranslLeft = -rightV - slaveV;
	Vector3 topTranslRight = rightV - slaveV;
	top_rect.translate(toRight ? topTranslRight : topTranslLeft);
	samples << base_rect.get2DRectangle(top_rect).getConners();

	// compute 2D bounding box
	Geom::Rectangle2& regionProj = toRight ? regionProjRight : regionProjLeft;
	Geom::Segment2 baseJoint2 = base_rect.get2DSegment(chains.front()->baseJoint);
	regionProj = Geom::computeBoundingBox(samples, baseJoint2.Direction);
}


void ZUnitScaff::setNbSplits(int n)
{
	UnitScaff::setNbSplits(n);
	hUnit->setNbSplits(n);
}

void ZUnitScaff::setNbChunks(int n)
{
	UnitScaff::setNbChunks(n);
	hUnit->setNbChunks(n);
}

void ZUnitScaff::setAabbCstr(Geom::Box aabb)
{
	UnitScaff::setAabbCstr(aabb);
	hUnit->setAabbCstr(aabb);
}

void ZUnitScaff::setImportance(double imp)
{
	UnitScaff::setImportance(imp);
	hUnit->setImportance(imp);
}

void ZUnitScaff::setCostWeight(double w)
{
	UnitScaff::setCostWeight(w);
	hUnit->setCostWeight(w);
}

void ZUnitScaff::setThickness(double thk)
{
	UnitScaff::setThickness(thk);
	hUnit->setThickness(thk);
}

bool ZUnitScaff::foldabilizeZ(SuperShapeKf* ssKeyframe, TimeInterval ti)
{
	// time interval
	mFoldDuration = ti;

	// obstacles
	obstacles = computeObstaclePnts(ssKeyframe, baseMaster->mID, topMaster->mID);

	// projected coordinates on the base rect
	baseRect = getBaseRect(ssKeyframe);
	QVector<Vector2> obstPntsProj;
	for (Vector3 s : obstacles)
		obstPntsProj << baseRect.getProjCoordinates(s);

	// projected aabb constraint on the base rect
	Geom::Rectangle2 aabbCstrProj = getAabbCstrProj(baseRect);

	// feasibility of left solution
	bool isCollidingLeft = regionProjLeft.containsAny(obstPntsProj, -0.01);
	bool inAABBLeft = aabbCstrProj.containsAll(regionProjLeft.getConners(), 0.01);
	fold2Left = !isCollidingLeft && inAABBLeft;

	// feasibility of right solution
	if (!fold2Left)
	{
		bool isCollidingRight = regionProjRight.containsAny(obstPntsProj, -0.01);
		bool inAABBRight = aabbCstrProj.containsAll(regionProjRight.getConners(), 0.01);
		fold2Right = !isCollidingRight && inAABBRight;
	}

	// apply Z solution : choose any
	if (fold2Left || fold2Right){
		QVector<FoldOption*>& options = fold2Left ? optionsLeft : optionsRight;
		for (int i = 0; i < chains.size(); i++)
			chains[i]->applyFoldOption(options[i]);
		return true;
	}
	else return false;
}

double ZUnitScaff::foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti)
{
	// fold as Z if possible, otherwise fold as H
	if (foldabilizeZ(ssKeyframe, ti)) return 0;
	else return hUnit->foldabilize(ssKeyframe, ti);
}


Scaffold* ZUnitScaff::getZKeyframe(double t, bool useThk)
{
	// base master
	Scaffold* kf = new Scaffold();
	kf->Structure::Graph::addNode(baseMaster->clone());

	// top master
	TChainScaff* chain = (TChainScaff*)chains.front();
	Vector3 uV = chain->upSeg.Direction;
	Vector3 rV = chain->rightDirect; if (fold2Left) rV *= -1;

	double L = chain->slaveSeg.length();
	double angle = chain->getRootAngle() * (1 - t);
	Vector3 R = L * cos(angle) * rV;
	Vector3 U = L * sin(angle) * uV;
	Vector3 S = chain->slaveSeg.P1 - chain->slaveSeg.P0;

	ScaffNode* tm = (ScaffNode*)topMaster->clone();
	tm->translate(R + U - S);
	kf->Structure::Graph::addNode(tm);

	// slaves
	for (int i = 0; i < chains.size(); i++)
	{
		ChainScaff* chain = chains[i];
		double localT = chain->duration.getLocalTime(t);
		Scaffold* ck = chain->getKeyframe(localT, useThk);
		ScaffNode* fSlave = ck->getScaffNode(chain->origSlave->mID);
		if (fSlave)	kf->Structure::Graph::addNode(fSlave->clone());
		delete ck;
	}

	// the key frame
	return kf;
}

Scaffold* ZUnitScaff::getKeyframe(double t, bool useThk)
{
	if (fold2Left || fold2Right)
		return getZKeyframe(t, useThk);
	else
		return hUnit->getKeyframe(t, useThk);
}

QVector<Vector3> ZUnitScaff::getCurrObstacles()
{
	if (fold2Left || fold2Right)
		return obstacles;
	else
		return hUnit->getCurrObstacles();
}

QVector<Geom::Rectangle> ZUnitScaff::getCurrAFRs()
{
	if (fold2Left || fold2Right)
	{
		QVector<Geom::Rectangle> afr;
		Geom::Rectangle2 regionProj = fold2Left ? regionProjLeft : regionProjRight;
		afr << baseRect.get3DRectangle(regionProj);
		return afr;
	}
	else
		return hUnit->getCurrAFRs();
}

QVector<Geom::Rectangle> ZUnitScaff::getCurrSlnFRs()
{
	if (fold2Left || fold2Right)
		return getCurrAFRs();
	else
		return hUnit->getCurrSlnFRs();
}

QVector<QString> ZUnitScaff::getSlnSlaveParts()
{
	if (fold2Left || fold2Right)
		return UnitScaff::getSlnSlaveParts();
	else
		return hUnit->getSlnSlaveParts();
}
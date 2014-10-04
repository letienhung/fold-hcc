#pragma once

#include "UnitScaff.h"
#include "HUnitScaff.h"

class ZUnitScaff final: public UnitScaff
{
public:
	ZUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
		QVector< QVector<QString> >& mPairs);

private:
	// decompose
	void sortMasters();
	void createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs);

	// two possible fold solutions
	void computeFoldSolution(Vector3 rV);
	void computeFoldRegionProj(bool isRight);

	// key frame as Z
	Scaffold* getKeyframeAsZ(double t, bool useThk);

	// obstacles
	QVector<Vector2> computeObstacles(ShapeSuperKeyframe* ssKeyframe);

	// foldabilize as Z and returns the true if success
	bool foldabilizeAsZ(ShapeSuperKeyframe* ssKeyframe);

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilization
	virtual double foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe) override;
	virtual void applySolution() override;

	// setter
	virtual void setImportance(double imp) override;

private:
	// the back up HUnit
	HUnitScaff* hUnit;

	// the top master
	PatchNode* topMaster;

	// two possible fold solutions
	// the right direction is not necessarily the right direction of each chain
	// it is only used to distinguish the two directions a ZUnit can be folded
	bool fold2Right, fold2Left;
	QVector<FoldOption*> leftOptions, rightOptions;
	Geom::Rectangle2 leftRegionProj, rightRegionProj;

};

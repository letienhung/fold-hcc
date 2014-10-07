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
	void computeFoldSolution(bool toRight);
	void computeFoldRegionProj(bool toRight);

	// key frame as Z
	Scaffold* getZKeyframe(double t, bool useThk);

	// obstacles
	QVector<Vector2> computeObstacles(ShapeSuperKeyframe* ssKeyframe);

	// foldabilize as Z and returns the true if success
	bool foldabilizeZ(ShapeSuperKeyframe* ssKeyframe, TimeInterval ti);

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilization
	virtual double foldabilize(ShapeSuperKeyframe* ssKeyframe, TimeInterval ti) override;

	// setter
	virtual void setImportance(double imp) override;

private:
	// the back up HUnit
	HUnitScaff* hUnit;

	// the top master
	PatchNode* topMaster;

	// two possible fold solutions
	// right direction is defined as the rightDirect of the first chain
	bool					fold2Left,		fold2Right;
	QVector<FoldOption*>	optionsLeft,	optionsRight;
	Geom::Rectangle2		regionProjLeft, regionProjRight;
};

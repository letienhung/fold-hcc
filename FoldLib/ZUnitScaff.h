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
	void prepareFoldSolution(bool toRight);
	void computeFoldRegionProj(bool toRight);

	// key frame as Z
	Scaffold* getZKeyframe(double t, bool useThk);

	// foldabilize as Z and returns the true if success
	bool foldabilizeZ(SuperShapeKf* ssKeyframe, TimeInterval ti);

public:
	// setter
	virtual void setImportance(double imp) override;

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// reset all fold options
	virtual void initFoldSolution() override;

	// foldabilization
	virtual double foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti) override;

	// debug
	virtual QVector<Vector3> getCurrObstacles() override;
	virtual QVector<Geom::Rectangle> getCurrAFRs() override;
	virtual QVector<Geom::Rectangle> getCurrSlnFRs() override;
	virtual QVector<QString> getSlnSlaveParts() override;

public:
	// the back up HUnit
	HUnitScaff* hUnit;

	// the top master
	PatchNode* topMaster;

	// two possible fold solutions
	// right direction is defined as the rightDirect of the first chain
	bool					fold2Left,		fold2Right;
	QVector<FoldOption*>	optionsLeft,	optionsRight;
	Geom::Rectangle2		regionProjLeft, regionProjRight;

	// the up-to-date base rect and obstacles
	Geom::Rectangle baseRect;
	QVector<Vector3> obstacles;
};

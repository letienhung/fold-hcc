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

	// possible fold regions
	void computeFoldRegions();

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilization
	virtual double foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe) override;
	virtual void applySolution() override;

private:
	// the back up HUnit
	HUnitScaff* hUnit;

	// the top master
	PatchNode* topMaster;

	// two possible fold regions
	Vector3 rightV;
	Geom::Rectangle2 leftRegionProj, rightRegionProj;
};

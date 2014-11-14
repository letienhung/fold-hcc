#pragma once

#include "ChainScaff.h"

class HChainScaff final : public ChainScaff
{
public:
	HChainScaff(ScaffNode* slave, PatchNode* base, PatchNode* top);

	// basic orientations
	virtual void computeOrientations() override;

	// fold option
	virtual Geom::Rectangle getFoldRegion(FoldOption* fn) override;

	// fold options
	virtual QVector<FoldOption*> genRegularFoldOptions() override;

	// cut planes
	virtual QVector<Geom::Plane> generateCutPlanes(FoldOption* fn) override;

	// fold
	virtual void fold(double t) override;
	void foldUniformHeight(double t);
	void foldUniformAngle(double t);
	void computePhaseSeparator();


public:
	// phase separator
	double				heightSep;	// the height separates phase I and II
	double				angleSep;	// angle between b and base

	// uniform option
	bool useUniformHeight;
};



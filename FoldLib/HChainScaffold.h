#pragma once

#include "ChainScaffold.h"

class HChainScaffold : public ChainScaffold
{
public:
	HChainScaffold(ScaffoldNode* slave, PatchNode* base, PatchNode* top);

	// fold option
	virtual Geom::Rectangle getFoldRegion(FoldOption* fn) override;

	// fold options
	virtual QVector<FoldOption*> genRegularFoldOptions(int nSplits, int nChunks) override;

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
	double				angleSep;	// angle between b and basez

	// uniform option
	bool useUniformHeight;
};



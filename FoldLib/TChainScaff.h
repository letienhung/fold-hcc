#pragma once

#include "ChainScaff.h"

class TChainScaff : public ChainScaff
{
public:
	TChainScaff(ScaffNode* slave, PatchNode* base, PatchNode* top);

	// override functions
	virtual Geom::Rectangle getFoldRegion(FoldOption* fn) override;

	// fold options
	virtual QVector<FoldOption*> genRegularFoldOptions(int nSplits, int nChunks) override;

	// cut planes
	virtual QVector<Geom::Plane> generateCutPlanes(FoldOption* fn) override;

	// fold
	virtual void fold(double t) override;
};

#pragma once

#include <ChainGraph.h>

class HChainGraph : public ChainGraph
{
public:
	HChainGraph(FdNode* slave, PatchNode* base, PatchNode* top);

	// fold option
	virtual Geom::Rectangle2 getFoldRegion(FoldOption* fn) override;

	// fold options
	virtual QVector<FoldOption*> genFoldOptions(int nSplits, int nChunks) override;

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



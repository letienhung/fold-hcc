#pragma once

#include "BlockGraph.h"
class TChainGraph;

class TBlockGraph : public BlockGraph
{
public:
	TBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
		QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb);

	// key frame
	virtual FdGraph* getKeyframe(double t, bool useThk) override;

	// folding region and volume
	virtual void computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double getAvailFoldingVolume() override;

	// foldabilize
	virtual void foldabilize(ShapeSuperKeyframe* ssKeyframe) override;

	// solution
	virtual void applySolution(int idx) override;

private:
	// folding regions
	void computeMinFoldingRegion(bool isRight);
	void computeMaxFoldingRegion(bool isRight);
	void computeAvailFoldingRegion(bool isRight, ShapeSuperKeyframe* ssKeyframe);

	// choose side
	Geom::Rectangle2& getMinFR(bool isRight);
	Geom::Rectangle2& getMaxFR(bool isRight);
	Geom::Rectangle2& getAvailFR(bool isRight);
	bool& getAbleToFoldTag(bool isRight);

	// height of the block
	double getHeight();

public:
	// pointer
	TChainGraph* tChain;
	PatchNode* topMaster;

	// folding regions
	Geom::Rectangle2 minFoldingRegionLeft, minFoldingRegionRight;
	Geom::Rectangle2 maxFoldingRegionLeft, maxFoldingRegionRight;
	Geom::Rectangle2 availFoldingRegionLeft, availFoldingRegionRight;

	// able to fold tag
	bool ableToFoldRight, ableToFoldLeft;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



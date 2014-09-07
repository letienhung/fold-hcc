#pragma once

#include "BlockGraph.h"

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

public:
	// the virtual top master
	PatchNode* topMaster;
};



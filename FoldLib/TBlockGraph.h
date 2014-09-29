#pragma once

#include "BlockGraph.h"
class TChainGraph;

class TBlockGraph : public BlockGraph
{
public:
	TBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
		QVector< QVector<QString> >& mPairs);

	// key frame
	virtual FdGraph* getKeyframe(double t, bool useThk) override;

	// foldabilize
	virtual QVector<int> getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double findOptimalSolution(const QVector<int>& afo) override;

private:
	QVector<Vector2> computeObstacles(ShapeSuperKeyframe* ssKeyframe);

public:
	// pointer
	TChainGraph* tChain;
	PatchNode* topMaster;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



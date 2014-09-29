#pragma once

#include "UnitScaffold.h"
class TChainScaffold;

class TUnitScaffold : public UnitScaffold
{
public:
	TUnitScaffold(QString id, QVector<PatchNode*>& ms, QVector<ScaffoldNode*>& ss,
		QVector< QVector<QString> >& mPairs);

	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilize
	virtual QVector<int> getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double findOptimalSolution(const QVector<int>& afo) override;

private:
	QVector<Vector2> computeObstacles(ShapeSuperKeyframe* ssKeyframe);

public:
	// pointer
	TChainScaffold* tChain;
	PatchNode* topMaster;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



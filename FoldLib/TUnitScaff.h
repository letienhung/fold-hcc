#pragma once

#include "UnitScaff.h"
class TChainScaff;

class TUnitScaff : public UnitScaff
{
public:
	TUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
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
	TChainScaff* tChain;
	PatchNode* topMaster;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



#pragma once

#include "UnitScaff.h"
class TChainScaff;

class TUnitScaff final: public UnitScaff
{
public:
	TUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
		QVector< QVector<QString> >& mPairs);

private:
	// decompose
	void sortMasters();
	void createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs);

	// obstacles
	QVector<Vector2> computeObstacles(ShapeSuperKeyframe* ssKeyframe);

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilize
	virtual QVector<int> getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double findOptimalSolution(const QVector<int>& afo) override;

public:
	// pointer
	TChainScaff* tChain;
	PatchNode* topMaster;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



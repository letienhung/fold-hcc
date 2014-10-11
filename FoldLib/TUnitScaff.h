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
	QVector<Vector2> computeObstacles(SuperShapeKf* ssKeyframe, UnitSolution* sln);

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilize
	virtual void computeAvailFoldOptions(SuperShapeKf* ssKeyframe, UnitSolution* sln) override;
	virtual void findOptimalSolution(UnitSolution* sln) override;

public:
	// pointer
	TChainScaff* tChain;
	PatchNode* topMaster;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



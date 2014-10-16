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

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// reset all fold options
	virtual void initFoldSolution() override;

	// foldabilize
	virtual double foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti) override;

	// debug info from the current solution
	virtual QVector<Vector3> getCurrObstacles() override;
	virtual QVector<Geom::Rectangle> getCurrAFRs() override;
	virtual QVector<Geom::Rectangle> getCurrSlnFRs() override;

public:
	// pointer
	TChainScaff* tChain;
	PatchNode* topMaster;

	// the up-to-date base rect and obstacles
	Geom::Rectangle baseRect;
	QVector<Vector3> obstacles;

	// all fold options are sorted based on their cost
	QVector<FoldOption*> sortedFoldOptions;

	// solution: the index in all fold options
	FoldOption* solution;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};



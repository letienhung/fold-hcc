#pragma once

#include "UnitScaff.h"
#include "FoldOptGraph.h"

class HUnitScaff final: public UnitScaff
{
public:
	HUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
		QVector< QVector<QString> >& mPairs);
	~HUnitScaff();

private:
	// decompose
	void sortMasters();
	void createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs);

	// obstacles
	void computeObstacles(SuperShapeKf* ssKeyframe, UnitSolution* sln);

	// collision graph
	FoldOptGraph* createCollisionGraph(const QVector<int>& afo);
	QVector<FoldOption*> findOptimalSolution(FoldOptGraph* collFog, const QVector<Structure::Node*>& component);
	QVector< QVector<bool> > genDualAdjMatrix(FoldOptGraph* collFog, const QVector<FoldOption*>& fns);
	QVector<double> genReversedWeights(const QVector<FoldOption*>& fns);

	// obstacles
	QVector<Vector3> getObstaclePoints();

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilization
	virtual void computeAvailFoldOptions(SuperShapeKf* ssKeyframe, UnitSolution* sln) override;
	virtual void findOptimalSolution(UnitSolution* sln) override;

public:
	// master-chain relation
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<int, QString> chainTopMasterMap;

	// timing 
	QVector<QString> sortedMasters; // bottom-up
	QMap<QString, double> masterTimeStamps;

	// obstacles
	QMap< QString, QVector<Vector2> > obstacles;
};


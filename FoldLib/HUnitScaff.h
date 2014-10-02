#pragma once

#include "UnitScaff.h"
#include "FoldOptionGraph.h"

class HUnitScaff : public UnitScaff
{
public:
	HUnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
		QVector< QVector<QString> >& mPairs);
	~HUnitScaff();

private:
	// H chains
	void createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs);
	void computeChainWeights();

	// obstacles
	void computeObstacles(ShapeSuperKeyframe* ssKeyframe);

	// collision graph
	FoldOptionGraph* createCollisionGraph(const QVector<int>& afo);
	QVector<FoldOption*> findOptimalSolution(FoldOptionGraph* collFog, const QVector<Structure::Node*>& component);
	QVector< QVector<bool> > genDualAdjMatrix(FoldOptionGraph* collFog, const QVector<FoldOption*>& fns);
	QVector<double> genReversedWeights(const QVector<FoldOption*>& fns);

	// obstacles
	QVector<Vector3> getObstaclePoints();

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// foldabilization
	virtual QVector<int> getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double findOptimalSolution(const QVector<int>& afo) override;

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


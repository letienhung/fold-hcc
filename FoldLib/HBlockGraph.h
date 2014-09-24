#pragma once

#include "BlockGraph.h"
#include "FoldOptionGraph.h"

class HBlockGraph : public BlockGraph
{
public:
	HBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
		QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb);
	~HBlockGraph();

private:
	// H chains
	void createChains(QVector<FdNode*>& ss, QVector< QVector<QString> >& mPairs);
	void computeChainWeights();

	// obstacles
	void computeObstacles(ShapeSuperKeyframe* ssKeyframe);

	// collision graph
	FoldOptionGraph* createCollisionGraph(const QVector<int>& afo);
	QVector<FoldOption*> findOptimalSolution(FoldOptionGraph* collFog, const QVector<Structure::Node*>& component);
	QVector< QVector<bool> > genDualAdjMatrix(FoldOptionGraph* collFog, const QVector<FoldOption*>& fns);
	QVector<double> genReversedWeights(const QVector<FoldOption*>& fns);

public:
	// key frame
	virtual FdGraph* getKeyframe(double t, bool useThk) override;

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


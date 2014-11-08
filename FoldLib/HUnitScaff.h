#pragma once

#include "UnitScaff.h"
#include "FoldOptGraph.h"

class HUnitSolution
{
public:
	Geom::Rectangle baseRect;		// the up-to-date position of the baseMaster
	Geom::Rectangle2 aabbCstrProj;	// the projection of aabb constraint on the base master
	QVector<int> afoIndices;		// indices of available fold options 
	QVector<int> chainSln;			// the solution (fold option index) for each chain
	double cost;					// the cost

	// debug
	QVector<Vector3> obstacles;		// obstacles sampled from obstacle parts

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

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

	// key steps for foldabilization
	void computeObstacles(SuperShapeKf* ssKeyframe, HUnitSolution* sln);
	void computeAvailFoldOptions(SuperShapeKf* ssKeyframe, HUnitSolution* sln);
	bool findExistedSolution(HUnitSolution* sln);
	void findOptimalSolution(HUnitSolution* sln);

	// collision graph
	FoldOptGraph* createCollisionGraph(const QVector<int>& afo);
	QVector<FoldOption*> findOptimalSolution(FoldOptGraph* collFog, const QVector<Structure::Node*>& component);
	QVector< QVector<bool> > genDualAdjMatrix(FoldOptGraph* collFog, const QVector<FoldOption*>& fns);
	QVector<double> genReversedWeights(const QVector<FoldOption*>& fns);

public:
	// key frame
	virtual Scaffold* getKeyframe(double t, bool useThk) override;

	// reset all fold options
	virtual void initFoldSolution() override;

	// foldabilization: the best solution is indicated by currSlnIdx
	// all tested set of avail fold options with the corresponding solution are also stored to avoid repeated computation
	virtual double foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti) override;

	// debug info from the current solution
	virtual QVector<Vector3> getCurrObstacles() override;
	virtual QVector<Geom::Rectangle> getCurrAFRs() override;
	virtual QVector<Geom::Rectangle> getCurrSlnFRs() override;

public:
	// master-chain relation
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<int, QString> chainTopMasterMap;

	// timing 
	QVector<QString> sortedMasters; // bottom-up
	QMap<QString, double> masterTimeStamps;

	// all fold options
	QVector<FoldOption*> allFoldOptions;

	// obstacles
	QMap< QString, QVector<Vector2> > tmObstaclesProj;

	// tested solutions
	int currSlnIdx;
	QVector<HUnitSolution*> testedSlns;
};


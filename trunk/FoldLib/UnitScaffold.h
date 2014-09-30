#pragma once

#include "Scaffold.h"
#include "ShapeSuperKeyframe.h"

class FoldOption;
class ChainScaffold;

class UnitScaffold : public Scaffold
{
public:
	UnitScaffold(QString id);
	~UnitScaffold();

	// selection
	ChainScaffold* getSelChain();
	Scaffold* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// set aabb constraint
	void setAabbConstraint(Geom::Box aabb);

	// #top masters: decides the folding duration 
	double	getNbTopMasters();

	// the total area of slave patches
	double getChainArea();

	// all fold options
	void genAllFoldOptions();

	// thickness
	void setThickness(double thk);

	// helper
	QVector<QString> getInbetweenExternalParts(Vector3 base_center, Vector3 top_center, ShapeSuperKeyframe* ssKeyframe);

	// visualize intermediate stuff
	void showObstaclesAndFoldOptions();

	// reset all fold options
	void resetAllFoldOptions();

public:
	//*** CORE
	// key frame
	virtual Scaffold*	getKeyframe(double t, bool useThk) = 0; // intermediate config. at local time t
	Scaffold*			getSuperKeyframe(double t);	// key frame with super master that merges all collapsed masters

	/* foldabilization : compute the best fold solution wrt the given super shape key frame
	   the best solution is indicated by currSlnIdx
	   all tested set of avail fold options with their fold solutions are also stored to avoid repeated computation	*/
	double					foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe); // foldabilize a block wrt. the context and returns the cost 
	virtual QVector<int>	getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe) = 0; // prune fold options wrt. to obstacles
	int						searchForExistedSolution(const QVector<int>& afo); // search for existed solution 
	virtual double			findOptimalSolution(const QVector<int>& afo) = 0; // store the optimal solution and returns the cost
	double					computeCost(FoldOption* fo);

	// apply the current solution (currSlnIdx)
	void applySolution();

public:
	//*** ENTITIES
	// chains
	int selChainIdx;
	QVector<ChainScaffold*> chains;
	QVector<double> chainWeights; // normalized area weights

	// masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;

public:
	//*** PARAMETERS
	// projected aabb constraint on the base master
	Geom::Rectangle2 aabbConstraint;

	// time
	double timeScale; 
	Interval mFoldDuration;

	// upper bound for modification
	int maxNbSplits;
	int maxNbChunks;

	// trade-off weight for computing cost
	double weight;

	// thickness
	bool useThickness;
	double thickness;

public:
	//*** SOLUTIONS
	// all fold options
	QVector<FoldOption*> allFoldOptions;

	// obstacles: projections on the base
	QVector<Vector2> obstacles;

	// sets of fold options that have been foldabilized
	QVector< QVector<int> > testedAvailFoldOptions;
	QVector< QVector<FoldOption*> > foldSolutions;
	QVector< double > foldCost;
	QVector< QVector<Vector3> > obstaclePnts;

	// the current fold solution (the latest solution generated by called foldabilizeWrt(*)
	// *** before any fold solution is applied, this index has to be correctly set
	int currSlnIdx;

	// tag
	bool foldabilized;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; 
#pragma once

#include "Scaffold.h"
#include "SuperShapeKf.h"
#include "TimeInterval.h"

class FoldOption;
class ChainScaff;

class UnitSolution
{
public:
	Geom::Rectangle baseRect;		// the up-to-date position of the baseMaster
	Geom::Rectangle2 aabbCstrProj;	// the projection of aabb constraint on the base master
	QVector<int> afoIndices;		// indices of available fold options 
	QVector<int> chainSln;			// the solution (fold option index) for each chain
	double cost;					// the cost

	// debug
	QVector<Vector3> obstacles;		// obstacles sampled from obstacle parts
	QVector<Vector3> obstaclesProj;	// obstacles projected on to the base rect

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class UnitScaff : public Scaffold
{
public:
	UnitScaff(QString id, QVector<PatchNode*>& ms, QVector<ScaffNode*>& ss,
		QVector< QVector<QString> >& mPairs);
	~UnitScaff();

	// chain importance wrt. patch area
	virtual void sortMasters() = 0;
	virtual void createChains(QVector<ScaffNode*>& ss, QVector< QVector<QString> >& mPairs) = 0;
	void computeChainImportances();

public:
	// selection
	ChainScaff* getSelChain();
	Scaffold* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();
	 
	// getters
	double getNbTopMasters();	// #top masters: decides the folding duration
	double getTotalSlaveArea(); // the total area of slave patches

	// parameter setters
	virtual void setNbSplits(int n);
	virtual void setNbChunks(int n);
	virtual void setAabbCstr(Geom::Box aabb);
	virtual void setCostWeight(double w);
	virtual void setImportance(double imp);
	virtual void setThickness(double thk);

	// debug info from the current solution
	virtual QVector<Vector3> getCurrObstacles();
	virtual QVector<Geom::Rectangle> getCurrAFRs();
	void genDebugInfo();

public:
	//*** CORE
	// key frame
	virtual Scaffold*	getKeyframe(double t, bool useThk) = 0; // intermediate config. at local time t
	Scaffold*			getSuperKeyframe(double t);	// key frame with super master that merges all collapsed masters

	// all fold options
	void genAllFoldOptions();
	void resetAllFoldOptions();

	// compute obstacles for a pair of masters
	bool isExternalPart(ScaffNode* snode);
	QVector<Vector3> computeObstaclePnts(SuperShapeKf* ssKeyframe, QString base_mid, QString top_mid);

	// the base rect
	Geom::Rectangle getBaseRect(SuperShapeKf* ssKeyframe);
	Geom::Rectangle2 getAabbCstrProj(SuperShapeKf* ssKeyframe);

	/* foldabilization : compute the best fold solution wrt the given super shape key frame
	   the best solution is indicated by currSlnIdx
	   all tested set of avail fold options with their fold solutions are also stored to avoid repeated computation	*/
	virtual double	foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti); // foldabilize a block wrt. the context and returns the cost 
	virtual void	computeAvailFoldOptions(SuperShapeKf* ssKeyframe, UnitSolution* sln); // prune fold options wrt. to obstacles
	virtual void	findOptimalSolution(UnitSolution* sln); // store the optimal solution and returns the cost

	// the cost of fold options
	double	computeCost(FoldOption* fo);

public:
	//*** ENTITIES
	// chains
	int selChainIdx;
	QVector<ChainScaff*> chains;

	// masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;

public:
	//*** PARAMETERS
	// aabb constraint
	Geom::Box aabbCstr;

	// time
	double timeScale; 
	TimeInterval mFoldDuration;

	// upper bound for modification
	int maxNbSplits;
	int maxNbChunks;

	// trade-off weight for computing cost
	double weight;

	// normalized importance wrt. patch area
	double importance; 

	// thickness
	bool useThickness;
	double thickness;

public:
	//*** SOLUTIONS
	// all fold options
	QVector<FoldOption*> allFoldOptions;

	// tested solutions
	int currSlnIdx;
	QVector<UnitSolution*> testedSlns;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; 
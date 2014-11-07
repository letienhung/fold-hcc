#pragma once

#include "Scaffold.h"
#include "SuperShapeKf.h"
#include "TimeInterval.h"

class FoldOption;
class ChainScaff;

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

	// importance
	virtual void setImportance(double imp);

	// debug info from the current solution
	virtual QVector<Vector3> getCurrObstacles() = 0;
	virtual QVector<Geom::Rectangle> getCurrAFRs() = 0;
	virtual QVector<Geom::Rectangle> getCurrSlnFRs() = 0;
	virtual QVector<QString> getSlnSlaveParts();
	void genDebugInfo();

public:
	//*** CORE
	// key frame
	virtual Scaffold*	getKeyframe(double t, bool useThk) = 0; // intermediate config. at local time t
	Scaffold*			getSuperKeyframe(double t);	// key frame with super master that merges all collapsed masters

	// initialize fold solution: generate all fold options and clear solutions
	virtual void initFoldSolution() = 0;

	// compute obstacles for a pair of masters
	bool isExternalPart(ScaffNode* snode);
	QVector<Vector3> computeObstaclePnts(SuperShapeKf* ssKeyframe, QString base_mid, QString top_mid);

	// the base rect
	Geom::Rectangle getBaseRect(SuperShapeKf* ssKeyframe);
	Geom::Rectangle2 getAabbCstrProj(Geom::Rectangle& base_rect);

	// foldabilization : search for the best fold solution wrt the given super shape key frame and return the cost
	virtual double	foldabilize(SuperShapeKf* ssKeyframe, TimeInterval ti) = 0;

public:
	// chains
	int selChainIdx;
	QVector<ChainScaff*> chains;

	// masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;

	// time
	double timeScale; 
	TimeInterval mFoldDuration;

	// normalized importance wrt. patch area
	double importance; 

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; 
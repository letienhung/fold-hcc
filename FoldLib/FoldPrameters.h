#pragma once
#include <QObject>
#include "FdUtility.h"

// this is a singleton class to hold all parameters

class FoldPrameters : public QObject
{
    Q_OBJECT

public:
	static FoldPrameters* getInstance();


private:
	// the singleton instance
	static FoldPrameters* _onlyInstance;

	// stop the compiler generating methods of copy the object
	FoldPrameters(const FoldPrameters&) = delete;
	FoldPrameters& operator=(const FoldPrameters&) = delete;

	// private constructor
	FoldPrameters();
	
public:
	// number of keyframes
	int nbKeyframes;

	// squeezing direction
	Vector3 sqzV;

	// aabb constraint
	Vector3 aabbCstrScale;

	// 
	int nbSplits;
	int nbChunks;

	// the trade-off weight for splitting
	double costWeight;

	// the connection ration
	double connThrRatio;
	double thickness;

signals:

public slots:

};


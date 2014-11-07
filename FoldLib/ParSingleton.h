#pragma once
#include <QObject>
#include "FdUtility.h"

// this is a singleton class to hold all parameters

class ParSingleton : public QObject
{
    Q_OBJECT

public:
	static ParSingleton* instance();


private:
	// stop the compiler generating methods of copy the object
	ParSingleton(const ParSingleton&) = delete;
	ParSingleton& operator=(const ParSingleton&) = delete;

	// private constructor
	ParSingleton();
	
public:
	// number of keyframes
	int nbKeyframes;

	// squeezing direction
	Vector3 sqzV;

	// aabb constraint
	Vector3 aabbCstrScale;
	Geom::Box aabbCstr;

	// upper bounds of modification
	int maxNbSplits;
	int maxNbChunks;

	// the trade-off weight for splitting
	double splitWeight;

	// the connection ration
	double connThrRatio;
	double thickness;

public:
	bool showDecomp;
	bool showKeyframe;

	bool showAABB;
	bool showCuboid;
	bool showScaffold;
	bool showMesh;

signals:
	void visualOptionChanged();
	void aabbCstrScaleChanged();

public slots:
	// foldabilization
	void setSqzV(QString sqzV_str);
	void setNbSplits(int N);
	void setNbChunks(int N);
	void setThickness(double thk);
	void setConnThrRatio(double thr);
	void setAabbX(double x);
	void setAabbY(double y);
	void setAabbZ(double z);
	void setCostWeight(double w);
	void setNbKeyframes(int N);

	// show options
	void setShowDecomp(int state);
	void setShowKeyframe(int state);
	void setShowAABB(int state);
	void setShowCuboid(int state);
	void setShowScaffold(int state);
	void setShowMesh(int state);
};


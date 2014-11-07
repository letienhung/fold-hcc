#include "ParSingleton.h"

ParSingleton* ParSingleton::instance()
{
	static ParSingleton* _onlyInstance = nullptr;

	if (!_onlyInstance)
	{
		_onlyInstance = new ParSingleton();
	}

	return _onlyInstance;
}


ParSingleton::ParSingleton()
{
	// foldabilization
	sqzV = Vector3(0, 0, 1);
	aabbCstrScale = Vector3(1, 1, 1);

	maxNbSplits = 1;
	maxNbChunks = 2;
	splitWeight = 0.5;

	connThrRatio = 0.07;
	thickness = 0;

	nbKeyframes = 50;

	// visual tags
	showKeyframe = false;
	showDecomp = false;
	showAABB = false;
	showCuboid = false;
	showScaffold = true;
	showMesh = false;
}

void ParSingleton::setSqzV(QString sqzV_str)
{
	sqzV = Vector3(0, 0, 0);
	if (sqzV_str == "X") sqzV[0] = 1;
	if (sqzV_str == "Y") sqzV[1] = 1;
	if (sqzV_str == "Z") sqzV[2] = 1;
	if (sqzV_str == "-X") sqzV[0] = -1;
	if (sqzV_str == "-Y") sqzV[1] = -1;
	if (sqzV_str == "-Z") sqzV[2] = -1;
}

void ParSingleton::setNbKeyframes(int N)
{
	nbKeyframes = N;
}

void ParSingleton::setNbSplits(int N)
{
	maxNbSplits = N;
}

void ParSingleton::setNbChunks(int N)
{
	maxNbChunks = N;
}

void ParSingleton::setThickness(double thk)
{
	thickness = thk;
}

void ParSingleton::setConnThrRatio(double thr)
{
	connThrRatio = thr;
}

void ParSingleton::setAabbX(double x)
{
	aabbCstrScale[0] = x;
}

void ParSingleton::setAabbY(double y)
{
	aabbCstrScale[1] = y;
}

void ParSingleton::setAabbZ(double z)
{
	aabbCstrScale[2] = z;
}

void ParSingleton::setCostWeight(double w)
{
	splitWeight = w;
}

void ParSingleton::setShowDecomp(int state)
{
	showDecomp = (state == Qt::Checked);
	emit(visualOptionChanged());
}

void ParSingleton::setShowAABB(int state)
{
	showAABB = (state == Qt::Checked);
	emit(visualOptionChanged());
}

void ParSingleton::setShowCuboid(int state)
{
	showCuboid = (state == Qt::Checked);
	emit(visualOptionChanged());
}

void ParSingleton::setShowScaffold(int state)
{
	showScaffold = (state == Qt::Checked);
	emit(visualOptionChanged());
}

void ParSingleton::setShowMesh(int state)
{
	showMesh = (state == Qt::Checked);
	emit(visualOptionChanged());
}

void ParSingleton::setShowKeyframe(int state)
{
	showKeyframe = (state == Qt::Checked);
	emit(visualOptionChanged());
}

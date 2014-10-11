#pragma once

#include "FoldOptGraph.h"

class UnitSolution
{
public:
    UnitSolution();

public:
	QVector<int> afo;
	QVector<FoldOption*> solution;
	double cost;

	QVector<Vector3> obstacles;
	QVector<Vector3> obstaclesProj;
};


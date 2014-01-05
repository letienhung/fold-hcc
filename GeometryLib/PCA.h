#pragma once

#include "UtilityGlobal.h"

namespace Geom{

class PCA
{
public:
    PCA(QVector<Vector3> &pnts);

	Vector3 mu;
	QVector<Vector3> eigenVectors;
	Vector3 eigenValues; 
};

}
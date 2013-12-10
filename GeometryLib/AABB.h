#pragma once
#include "Box.h"
#include "UtilityGlobal.h"

namespace Geom{

class AABB
{
public:
	AABB();
	AABB(QVector<Vector3>& pnts);
	AABB(SurfaceMeshModel* mesh);

	void buildFromPoints(QVector<Vector3>& pnts);
	void buildFromMesh(SurfaceMeshModel* mesh);

	Vector3 bbmin, bbmax;

	Vector3 center();
	double radius();
	Box box();
};

}
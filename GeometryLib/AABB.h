#pragma once
#include "Box.h"
#include "UtilityGlobal.h"

namespace Geom{

class AABB
{
public:
	AABB();
	AABB(QVector<Vector3>& pnts);

	void add(QVector<Vector3>& pnts);
	void add(AABB& other);

	Vector3 center();
	double radius();
	Box box();

	bool isValid();
	void validate();

public:
	Vector3 bbmin, bbmax;
};

}
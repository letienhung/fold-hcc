#pragma once

#include "UtilityGlobal.h"
#include "Segment.h"

namespace Geom{

class Line;

class Plane
{
public:
    Plane();
	Plane(Vector3 c, Vector3 n);

	double	signedDistanceTo(Vector3 p);
	double	distanceTo(Vector3 p);

	int		whichSide(Vector3 p);
	bool	onSameSide( QVector<Vector3>& pnts );

	bool	contains(Line line);

	Vector3 getProjection(Vector3 p);

	bool intersects(Segment seg);
	Vector3 getIntersection(Segment seg);

	Plane opposite();
	void flip();

	void translate(Vector3 t);
	Plane translated(Vector3 t);

	void draw(QColor c = Qt::green);

public:
	Vector3 Constant, Normal;
};

}
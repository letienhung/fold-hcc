#include "Rectangle2.h"

Geom::Rectangle2::Rectangle2()
{

}

Geom::Rectangle2::Rectangle2( Vector2 &center, QVector<Vector2> &axis, Vector2 &extent )
{
	Center = center;
	Axis = axis;
	Extent = extent;
}

Geom::Rectangle2::Rectangle2(QVector<Vector2> &conners)
{
	Center = Vector2(0,0);
	foreach(Vector2 p, conners) Center += p;
	Center /= 4;

	Vector2 e0 = conners[1] - conners[0];
	Vector2 e1 = conners[3] - conners[0];

	Axis.push_back(e0.normalized());
	Axis.push_back(e1.normalized());

	Extent = Vector2(e0.norm(), e1.norm()) / 2;
}

QVector<Vector2> Geom::Rectangle2::getConners()
{
	Vector2 extAxis0 = Axis[0] * Extent[0];
	Vector2 extAxis1 = Axis[1] * Extent[1];

	return QVector<Vector2>()
		<< Center - extAxis0 - extAxis1
		<< Center + extAxis0 - extAxis1
		<< Center + extAxis0 + extAxis1
		<< Center - extAxis0 + extAxis1;
}


QStringList Geom::Rectangle2::toStrList()
{
	return QStringList() << "Rectangle2: "
		<< "Center = " + qStr(Center)
		<< "Axis[0] = " + qStr(Axis[0])
		<< "Axis[1] = " + qStr(Axis[1])
		<< "Extent = " + qStr(Extent);
}
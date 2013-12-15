#include "Rectangle.h"
#include "Plane.h"
#include "Numeric.h"
#include "Segment2.h"
#include "CustomDrawObjects.h"

Geom::Rectangle::Rectangle()
{
	Center = Vector3(0,0,0);
	Axis << Vector3(1,0,0) << Vector3(0,1,0);
	Extent = Vector2(0.5, 0.5);

	Axis[0].normalize(); Axis[1].normalize();
	Normal = cross(Axis[0], Axis[1]).normalized();
}

Geom::Rectangle::Rectangle( QVector<Vector3>& conners )
{
	Center = Vector3(0, 0, 0);
	foreach (Vector3 p, conners) Center += p;
	Center /= 4;

	Vector3 e0 = conners[1] - conners[0];
	Vector3 e1 = conners[3] - conners[0];

	Axis.push_back(e0.normalized());
	Axis.push_back(e1.normalized());

	Extent = Vector2(e0.norm()/2, e1.norm()/2);

	Normal = cross(e0, e1).normalized();
}

Geom::Rectangle::Rectangle( Vector3& c, QVector<Vector3>& a, Vector2& e )
{
	Center = c;
	Axis = a;
	Extent = e;

	Axis[0].normalize(); Axis[1].normalize();
	Normal = cross(Axis[0], Axis[1]).normalized();
}

bool Geom::Rectangle::isCoplanarWith( Vector3 p )
{
	Plane plane(Center, Normal);
	return plane.whichSide(p) == 0;
}

bool Geom::Rectangle::isCoplanarWith( Segment s )
{
	Plane plane(Center, Normal);
	return (plane.whichSide(s.P0) == 0) 
		&& (plane.whichSide(s.P1) == 0);
}


bool Geom::Rectangle::isCoplanarWith( Rectangle& other )
{
	foreach (Vector3 p, other.getConners())
		if (!this->isCoplanarWith(p)) return false;

	return true;
}

Vector2 Geom::Rectangle::getCoordinates( Vector3 p )
{
	Vector3 v = p - Center;
	double x = dot(v, Axis[0])/Extent[0];
	double y = dot(v, Axis[1])/Extent[1];
	
	return Vector2(x, y);
}

bool Geom::Rectangle::contains( Vector3 p)
{
	if (!this->isCoplanarWith(p)) return false;

	Vector2 coord = this->getCoordinates(p);
	double threshold = 1 + ZERO_TOLERANCE_LOW;

	return (fabs(coord[0]) < threshold) 
		&& (fabs(coord[1]) < threshold);
}

bool Geom::Rectangle::contains( Segment s)
{
	return this->contains(s.P0) && this->contains(s.P1);
}

bool Geom::Rectangle::contains( Rectangle& other )
{
	foreach (Vector3 p, other.getConners())
		if (!this->contains(p)) return false;

	return true;
}

Geom::Plane Geom::Rectangle::getPlane()
{
	return Plane(this->Center, this->Normal);
}



QVector<Geom::Segment> Geom::Rectangle::getEdges()
{
	QVector<Segment> edges;

	QVector<Vector3> Conners = getConners();
	for (int i = 0; i < 4; i++)
		edges.push_back(Segment(Conners[i], Conners[(i+1)%4]));

	return edges;
}

QVector<Geom::Segment2> Geom::Rectangle::get2DEdges()
{
	QVector<Segment2> edges;

	QVector<Vector2> pnts = this->get2DConners();
	for (int i = 0; i < 4; i++)
		edges.push_back(Segment2(pnts[i], pnts[(i+1)%4]));

	return edges;
}

QVector<Vector2> Geom::Rectangle::get2DConners()
{
	QVector<Vector2> pnts;

	pnts.push_back(Vector2( 1,  1));
	pnts.push_back(Vector2(-1,  1));
	pnts.push_back(Vector2(-1, -1));
	pnts.push_back(Vector2( 1, -1));

	return pnts;
}

SurfaceMesh::Vector2 Geom::Rectangle::getProjCoordinates( Vector3 p )
{
	Vector2 coord;
	p = p - Center;
	for (int i = 0; i < 2; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

SurfaceMesh::Vector3 Geom::Rectangle::getPosition( const Vector2& c )
{
	Vector3 pos = Center;
	for (int i = 0; i < 2; i++)
		pos += c[i]* Extent[i] * Axis[i];

	return pos;
}

Geom::Segment2 Geom::Rectangle::getProjection2D( Segment s )
{
	Vector2 p0 = this->getProjCoordinates(s.P0);
	Vector2 p1 = this->getProjCoordinates(s.P1);

	return Segment2(p0, p1);
}

QVector<Vector3> Geom::Rectangle::getConners()
{
	QVector<Vector3> conners;
	Vector3 dx = Extent[0] * Axis[0];
	Vector3 dy = Extent[1] * Axis[1];
	conners.push_back(Center + dx + dy);
	conners.push_back(Center - dx + dy);
	conners.push_back(Center - dx - dy);
	conners.push_back(Center + dx - dy);
	
	return conners;
}

QVector<Vector3> Geom::Rectangle::getConnersReverse()
{
	QVector<Vector3> conners(4);
	Vector3 dx = Extent[0] * Axis[0];
	Vector3 dy = Extent[1] * Axis[1];
	conners[3] = Center + dx + dy;
	conners[2] = Center - dx + dy;
	conners[1] = Center - dx - dy;
	conners[0] = Center + dx - dy;

	return conners;
}

void Geom::Rectangle::draw(QColor color)
{
	PolygonSoup ps;
	ps.addPoly(getConners(), color);
	ps.drawQuads();
}



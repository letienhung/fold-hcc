#include "Rectangle.h"
#include "Plane.h"
#include "Numeric.h"
#include "Segment2.h"
#include "CustomDrawObjects.h"


//   1	 _______e2______  0
//		|				|
//		|		Y		|
//		|		|		|
//	e1	|		|___ X	|e0
//		|				|
//		|				|
//	   2|_______________|3
//				e3

int Geom::Rectangle::EDGE[4][2] = {
	3, 0,
	2, 1,
	1, 0,
	2, 3
};

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

	if (e0.norm() == 0)
		Axis.push_back(Vector3(0, 0, 0));
	else
		Axis.push_back(e0.normalized());

	if (e1.norm() == 0)
		Axis.push_back(Vector3(0, 0, 0));
	else
		Axis.push_back(e1.normalized());

	Extent = Vector2(e0.norm()/2, e1.norm()/2);

	Vector3 crosse0e1 = cross(e0, e1);
	if (crosse0e1.norm() == 0)
		Normal = Vector3(0, 0, 0);
	else
		Normal = crosse0e1.normalized();
}

Geom::Rectangle::Rectangle( Vector3& c, QVector<Vector3>& a, Vector2& e )
{
	Center = c;
	Axis = a;
	Extent = e;

	Axis[0].normalize(); Axis[1].normalize();
	Normal = cross(Axis[0], Axis[1]).normalized();
}

Geom::Rectangle::Rectangle(const Rectangle &r)
{
	Center = r.Center;
	Axis = r.Axis;
	Extent = r.Extent;
	Normal = r.Normal;
}


bool Geom::Rectangle::isCoplanarWith( Vector3 p )
{
	Plane plane(Center, Normal);
	return plane.whichSide(p) == 0;
}

bool Geom::Rectangle::isCoplanarWith( Rectangle& other )
{
	foreach (Vector3 p, other.getConners())
		if (!this->isCoplanarWith(p)) return false;

	return true;
}

bool Geom::Rectangle::contains( Vector3 p)
{
	if (!this->isCoplanarWith(p)) return false;

	Vector2 coord = this->getProjCoordinates(p);
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

bool Geom::Rectangle::containsOneEdge( Rectangle& other )
{
	foreach (Segment e, other.getEdgeSegments())
		if (contains(e)) return true;

	return false;
}

Geom::Plane Geom::Rectangle::getPlane()
{
	return Plane(this->Center, this->Normal);
}

QVector<Geom::Segment> Geom::Rectangle::getEdgeSegments()
{
	QVector<Segment> edges;

	QVector<Vector3> Conners = getConners();
	for (int i = 0; i < 4; i++)
		edges.push_back(Segment(Conners[ EDGE[i][0] ], Conners[ EDGE[i][1] ]));

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


Vector2 Geom::Rectangle::getProjCoordinates( Vector3 p )
{
	Vector3 v = p - Center;
	double x = dot(v, Axis[0])/Extent[0];
	double y = dot(v, Axis[1])/Extent[1];

	return Vector2(x, y);
}

Vector3 Geom::Rectangle::getPosition( const Vector2& c )
{
	Vector3 pos = Center;
	for (int i = 0; i < 2; i++)
		pos += c[i]* Extent[i] * Axis[i];

	return pos;
}

Vector3 Geom::Rectangle::getVector( const Vector2& v )
{
	return getPosition(v) - Center;
}

Vector2 Geom::Rectangle::getOpenProjCoord( Vector3 p )
{
	Vector3 v = p - Center;
	double x = dot(v, Axis[0]);
	double y = dot(v, Axis[1]);

	return Vector2(x, y);
}


Vector3 Geom::Rectangle::getOpenVector( const Vector2& v )
{
	return getOpenPos(v) - Center;
}


Vector3 Geom::Rectangle::getOpenPos( const Vector2& c )
{
	Vector3 pos = Center;
	for (int i = 0; i < 2; i++)
		pos += c[i]*Axis[i];

	return pos;
}



Geom::Segment2 Geom::Rectangle::get2DSegment( Segment& s )
{
	Vector2 p0 = getProjCoordinates(s.P0);
	Vector2 p1 = getProjCoordinates(s.P1);

	return Segment2(p0, p1);
}


Geom::Segment Geom::Rectangle::get3DSegment( Segment2& s )
{
	Vector3 p0 = getPosition(s.P0);
	Vector3 p1 = getPosition(s.P1);

	return Segment(p0, p1);
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

void Geom::Rectangle::drawFace( QColor color /*= Qt::red*/ )
{
	PolygonSoup ps;
	ps.addPoly(getConners(), color);
	ps.drawQuads();
}

void Geom::Rectangle::drawBackFace( QColor color /*= Qt::red*/ )
{
	QVector<Vector3> pnts = getConnersReverse();
	for (int i = 0; i < 4; i++) 
		pnts[i] -= Extent[0] * (10e-4) * Normal;

	PolygonSoup ps;
	ps.addPoly(pnts, color);
	ps.drawQuads();
}

void Geom::Rectangle::drawEdges( double width /*= 2.0*/, QColor color /*= Qt::red*/ )
{
	LineSegments ls(width);
	foreach (Segment e, getEdgeSegments())
		ls.addLine(e.P0, e.P1, color);
	ls.draw();
}

double Geom::Rectangle::area()
{
	return 4 * Extent[0] * Extent[1];
}

QVector<Geom::Segment> Geom::Rectangle::getPerpEdges(Vector3 v)
{
	QVector<Segment> edges = getEdgeSegments();
	int aid = getClosestAxisId(v);

	return QVector<Segment>() << edges[2*aid] << edges[2*aid + 1];
}

QStringList Geom::Rectangle::toStrList()
{
	return QStringList() << "Rectangle: "
		<< "Center = " + qStr(Center)
		<< "Axis[0] = " + qStr(Axis[0])
		<< "Axis[1] = " + qStr(Axis[1])
		<< "Extent = " + qStr(Extent);
}

Vector3 Geom::Rectangle::getProjection( Vector3 p )
{
	return getPlane().getProjection(p);
}

SurfaceMesh::Vector3 Geom::Rectangle::getProjectedVector( Vector3 v )
{
	return getProjection(Center + v) - Center;
}

int Geom::Rectangle::getClosestAxisId( Vector3 v )
{
	double dotVAxis0 = fabs(dot(v, Axis[0]));
	double dotVAxis1 = fabs(dot(v, Axis[1]));

	return (dotVAxis0 > dotVAxis1) ? 0 : 1;
}


int Geom::Rectangle::getPerpAxisId( Vector3 v )
{
	double dotVAxis0 = fabs(dot(v, Axis[0]));
	double dotVAxis1 = fabs(dot(v, Axis[1]));

	return (dotVAxis0 > dotVAxis1) ? 1 : 0;
}

Geom::Rectangle Geom::Rectangle::getRectangle( Rectangle2 &rect2 )
{
	QVector<Vector3> conners;
	foreach(Vector2 p2, rect2.getConners()) 
		conners << getPosition(p2);

	return Rectangle(conners);
}

double Geom::Rectangle::radius()
{
	return sqrt(Extent[0] * Extent[0] + Extent[1] * Extent[1]);
}

QVector<Vector3> Geom::Rectangle::getEdgeSamples( int N )
{
	QVector<Vector3> samples;
	foreach(Geom::Segment edge, getEdgeSegments())
		samples += edge.getUniformSamples(N);

	return samples;
}

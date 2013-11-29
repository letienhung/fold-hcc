#include "Box.h"
#include "Numeric.h"

//		  7-----------6                     Y
//		 /|          /|                   f2^   /f5
//		4-+---------5 |                     |  / 
//		| |         | |                     | /
//		| |         | |             f1      |/     f0
//		| 3---------+-2            ---------+-------> X 
//		|/          |/                     /|
//		0-----------1                     / |
//								       f4/  |f3
//	

using namespace Geom;

int Box::NB_FACES = 6;
int Box::NB_EDGES = 12;
int Box::NB_VERTICES = 8;

int Box::EDGE[12][2] = {
	0, 1,
	2, 3,
	6, 7,
	4, 5,
	0, 4,
	1, 5,
	2, 6,
	3, 7,
	0, 3, 
	1, 2,
	5, 6,
	4, 7
};

int Box::QUAD_FACE[6][4] = 
{
	1, 2, 6, 5,
	0, 4, 7, 3,
	4, 5, 6, 7,
	0, 3, 2, 1,
	0, 1, 5, 4,
	2, 3, 7, 6
};

int Box::TRI_FACE[12][3] = 
{
	1, 2, 6,
	6, 5, 1,
	0, 4, 7,
	7, 3, 0,
	4, 5, 6,
	6, 7, 4,
	0, 3, 2,
	2, 1, 0,
	0, 1, 5,
	5, 4, 0,
	2, 3, 7,
	7, 6, 2
};

Box::Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext)
{
	Center = c;
	Axis = axis;
	Extent = ext;

	// normalize axis
	for (int i = 0; i < 3; i++)
		Axis[i].normalize();

	// right handed system
	if (dot(cross(Axis[0], Axis[1]) , Axis[2]) < 0)
		Axis[2] *= -1;
}

Box &Box::operator =(const Box &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;

    return *this;
}

Frame Box::getFrame()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
{
	return Frame(this->Center, this->Axis[0], this->Axis[1], this->Axis[2]);
}

void Box::setFrame( Frame f )
{
	this->Center = f.c;
	this->Axis[0] = f.r;
	this->Axis[1] = f.s;
	this->Axis[2] = f.t;
}

Vector3 Box::getCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

Vector3 Box::getPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Extent[i] * Axis[i];

	return pos;
}

void Box::translate( Vector3 t )
{
	this->Center += t;
}

void Box::scale( double s )
{
	this->Extent *= s;
}

void Box::scale( Vector3 s )
{
	for (int i = 0; i < 3; i++)	
		this->Extent[i] *= s[i];
}

bool Box::hasFaceCoplanarWith( Line line )
{
	Vector3 p1 = line.getPoint(0);
	Vector3 p2 = line.getPoint(1);

	foreach(Plane plane, this->getFacePlanes())
	{
		if (plane.contains(line)) return true;
	}

	return false;
}

QVector<Point> Box::getConnerPoints()
{
	QVector<Point> pnts(8);

	QVector<Vector3> edges;
	for (int i = 0; i < 3; i++)
		edges.push_back( 2 * Extent[i] * Axis[i]);

	pnts[0] = Center - 0.5*edges[0] - 0.5*edges[1] + 0.5*edges[2];
	pnts[1] = pnts[0] + edges[0];
	pnts[2] = pnts[1] - edges[2];
	pnts[3] = pnts[2] - edges[0];

	pnts[4] = pnts[0] + edges[1];
	pnts[5] = pnts[1] + edges[1];
	pnts[6] = pnts[2] + edges[1];
	pnts[7] = pnts[3] + edges[1];

	return pnts;
}

QVector<Line> Box::getEdgeLines()
{
	QVector<Line> lines;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 0; i < 12; i++)
	{
		lines.push_back(Line(pnts[ EDGE[i][0] ], pnts[ EDGE[i][1] ]));
	}

	return lines;
}

QVector<Segment> Box::getEdgeSegments()
{
	QVector<Segment> edges;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 0; i < 12; i++)
	{
		edges.push_back(Segment(pnts[ EDGE[i][0] ], pnts[ EDGE[i][1] ]));
	}

	return edges;
}

QVector< QVector<Point> > Box::getFacePoints()
{
	QVector< QVector<Point> > faces(6);

	QVector<Point> pnts = getConnerPoints();
	for (int i = 0; i < 6; i++)	{
		for (int j = 0; j < 4; j++)	{
			faces[i].push_back( pnts[ QUAD_FACE[i][j] ] );
		}
	}	

	return faces;
}

QVector<Plane> Box::getFacePlanes()
{
	QVector<Plane> faces;
	for (int i = 0; i < 3; i++)
	{
		Vector3 offset = Extent[i] * Axis[i];
		faces.push_back(Plane(Center+offset,  Axis[i]));
		faces.push_back(Plane(Center-offset, -Axis[i]));
	}
	return faces;
}

QVector<Rectangle> Box::getFaceRectangles()
{
	QVector<Rectangle> rects;

	foreach( QVector<Point> conners, this->getFacePoints() )
		rects.push_back(Rectangle(conners));

	return rects;
}

QVector<Segment> Box::getEdgeIncidentOnPoint(Point &p)
{
	QVector<Segment> edges;
	
	foreach(Segment s, this->getEdgeSegments()){
		if(s.P0 == p || s.P1 == p)
			edges.push_back(s);
	}

	return edges;
}
QVector<Rectangle> Box::getFaceIncidentOnPoint(Point &p)
{
	QVector<Rectangle> rects;

	foreach(Rectangle r, this->getFaceRectangles()){
		foreach(Point cp, r.Conners){
			if(p == cp)
				rects.push_back(r);
		}
	}

	return rects;
}

QVector<Vector3> Geom::Box::getGridSamples( int N )
{
	QVector<Vector3> samples;

	// the size of regular grid
	double gridV = this->getVolume() / N;
	double gridSize = pow(gridV, 1.0/3);
	int nbX = (int)ceil(2 * Extent[0] / gridSize);
	int nbY = (int)ceil(2 * Extent[1] / gridSize);
	int nbZ = (int)ceil(2 * Extent[2] / gridSize);
	double stepX = 2.0 / nbX;
	double stepY = 2.0 / nbY;
	double stepZ = 2.0 / nbZ;

	for (int i = 0; i <= nbX; i++)
	for (int j = 0; j <= nbY; j++)
	for (int k = 0; k <= nbZ; k++)
	{
		double ci = -1 + i * stepX;
		double cj = -1 + j * stepY;
		double ck = -1 + k * stepZ;
		samples.push_back(this->getPosition(Vector3(ci, cj, ck)));
	}

	return samples;
}

bool Geom::Box::contains( Vector3 p )
{
	Vector3 c = this->getCoordinates(p);
	return inRange(-1, 1, c.x())
		&& inRange(-1, 1, c.y())
		&& inRange(-1, 1, c.z());
}

double Geom::Box::getVolume()
{
	return 8 * Extent[0] * Extent[1] * Extent[2];
}

Geom::Box Geom::Box::scaled( double s )
{
	Box b = *this;
	b.scale(s);
	return b;
}


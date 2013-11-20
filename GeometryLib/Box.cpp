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
		coord[i] = dot(p, Axis[i]);

	return coord;
}

SurfaceMesh::Vector3 Box::getPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Axis[i];

	return pos;
}

SurfaceMesh::Vector3 Box::getUniformCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

SurfaceMesh::Vector3 Box::getUniformPosition( Vector3 coord )
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

void Box::uniformScale( double s )
{
	this->Extent *= s;
}

void Box::scale( Vector3 s )
{
	for (int i = 0; i < 3; i++)	
		this->Extent[i] *= s[i];
}

bool Box::onBox( Line line )
{
	Vector3 p1 = line.getPoint(0);
	Vector3 p2 = line.getPoint(1);

	foreach(Plane plane, this->getFacePlanes())
	{
		if (plane.whichSide(p1) == 0 && plane.whichSide(p2) == 0)
		{
			return true;
		}
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

QVector<Box2> Box::getFaceRectangles()
{
	QVector<Box2> rects;

	foreach( QVector<Point> conners, this->getFacePoints() )
		rects.push_back(Box2(conners));

	return rects;
}



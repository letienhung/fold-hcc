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
//								      f4|/  |f3
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

void Geom::Box::scale( int axisID, double s )
{
	if (axisID >= 0 && axisID < 3)
		this->Extent[axisID] *= s;
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


QVector<Segment> Geom::Box::getEdgeSegmentsAlongAxis( int aid )
{
	QVector<Segment> edges;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 4 * aid; i < 4 * aid + 4; i++)
	{
		edges.push_back(Segment(pnts[EDGE[i][0]], pnts[EDGE[i][1]]));
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
	return inRange(c.x(), -1, 1)
		&& inRange(c.y(), -1, 1)
		&& inRange(c.z(), -1, 1);
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

int Geom::Box::getFaceId( Vector3 n )
{
	for (int i = 0; i < 3; i++)	
	{
		if (areCollinear(n, Axis[i])) 
		{
			return (dot(n, Axis[i]) > 0) ? 
				(3*i) : (3*i + 1);
		}
	}

	return -1;
}


int Geom::Box::getAxisId( Vector3 a )
{
	for (int i = 0; i < 3; i++)	
		if (areCollinear(a, Axis[i])) return i;

	return -1;
}


double Geom::Box::calcFrontierWidth( int fid, const QVector<Vector3>& pnts, bool two_side /*= false*/ )
{
	int axisID = fid / 3;
	bool isNeg = fid % 3;

	double maxWidth = std::numeric_limits<double>::min();
	foreach(Vector3 p, pnts)
	{
		// off from face
		Vector3 coord = this->getCoordinates(p);
		double width = isNeg ? 
			1 + coord[axisID] : 1 - coord[axisID];

		// if two-side is enabled
		// skip points closer to the opposite face
		if (two_side && width > 1) continue;

		// update the maxWidth
		if (width > maxWidth) maxWidth = width;
	}

	return maxWidth;
}

SurfaceMesh::Vector4 Geom::Box::calcFrontierWidth( Vector3 hX, Vector3 hZ, const QVector<Vector3>& pnts )
{
	int hxId = getAxisId(hX), hzId = getAxisId(hZ);
	double xE = Extent[hxId], zE = Extent[hzId];

	double xlow = -1, xhigh = 1, zlow = -1, zhigh = 1;
	foreach(Vector3 p, pnts)
	{
		Vector3 coord = this->getCoordinates(p);
		double x = coord[hxId], z = coord[hzId];
		double xCost = 0, zCost = 0;

		// hx direction
		double new_xlow = xlow, new_xhigh = xhigh;
		if (inRange(x, xlow, xhigh))
		{
			double x_xlow = x - xlow, xhigh_x = xhigh - x;
			if (x_xlow <= xhigh_x) { 
				xCost = x_xlow * zE; new_xlow = x;
			}else{
				xCost = xhigh_x * zE; new_xhigh = x;
			}
		}

		// hz direction
		double new_zlow = zlow, new_zhigh = zhigh;
		if (inRange(z, zlow, zhigh))
		{
			double z_zlow = z - zlow, zhigh_z = zhigh - z;
			if (z_zlow <= zhigh_z) { 
				zCost = z_zlow * xE; new_zlow = z;
			}else{ 
				zCost = zhigh_z * xE; new_zhigh = z;
			}
		}

		// reduce range with lower cost
		if (xCost <= zCost){
			xlow = new_xlow; xhigh = new_xhigh;
		}else{
			zlow = new_zlow; zhigh = new_zhigh;
		}
	}

	// reverse direction if need
	if (dot(hX, Axis[hxId])){
		std::swap(xlow, xhigh); xlow *= -1; xhigh *= -1;
	}
	if (dot(hZ, Axis[hzId])){
		std::swap(zlow, zhigh); zlow *= -1; zhigh *= -1;
	}

	return Vector4(1 + xlow, 1 - xhigh, 1 + zlow, 1 - zhigh);
}

int Geom::Box::getType( double threshold )
{
	QVector<double> ext;
	ext << Extent[0] << Extent[1] << Extent[2];
	qSort(ext);

	if (ext[2] / ext[1] > threshold)
		return ROD;
	else if (ext[1] / ext[0] > threshold)
		return PATCH;
	else
		return BRICK;
}


int Geom::Box::minAxisId()
{
	int id = 0;
	double minExt = Extent[0];
	for (int i = 1; i < 3; i++)
	{
		if (Extent[i] < minExt)
		{
			minExt = Extent[i];
			id = i;
		}
	} 

	return id;	
}


int Geom::Box::maxAxisId()
{
	int id = 0;
	double maxExt = Extent[0];
	for (int i = 1; i < 3; i++)
	{
		if (Extent[i] > maxExt)
		{
			maxExt = Extent[i];
			id = i;
		}
	} 

	return id;	
}

SurfaceMesh::Vector3 Geom::Box::getFaceCenter( int fid )
{
	Vector3 c = Center;

	int aid = fid / 3;
	if (fid % 3)
		c -= Extent[aid] * Axis[aid];
	else
		c += Extent[aid] * Axis[aid];

	return c;
}


int Geom::Box::getFaceId( int aid, bool positive )
{
	return positive ? (3 * aid) : (3 * aid + 1);
}

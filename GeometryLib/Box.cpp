#include "Box.h"
#include "Numeric.h"
#include "CustomDrawObjects.h"

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

int Geom::Box::NB_FACES = 6;
int Geom::Box::NB_EDGES = 12;
int Geom::Box::NB_VERTICES = 8;

int Geom::Box::EDGE[12][2] = {
	0, 1,
	3, 2,
	7, 6,
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

int Geom::Box::QUAD_FACE[6][4] = 
{
	1, 2, 6, 5,
	0, 4, 7, 3,
	4, 5, 6, 7,
	0, 3, 2, 1,
	0, 1, 5, 4,
	2, 3, 7, 6
};

int Geom::Box::TRI_FACE[12][3] = 
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

Geom::Box::Box()
{
	Axis.resize(3);
}

Geom::Box::Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext)
{
	Center = c;
	Axis = axis;
	Extent = ext;

	// normalize axis
	for (int i = 0; i < 3; i++)
		Axis[i].normalize();

	// right handed system
	Vector3 cross01 = cross(Axis[0], Axis[1]);
	if (dot(cross01, Axis[2]) < 0)
		Axis[2] *= -1;
}

Geom::Box::Box(const Frame& f, const Vector3& ext)
{
	Center = f.c;
	Axis.clear();
	Axis << f.r << f.s << f.t;
	Extent = ext;
}


Geom::Box &Geom::Box::operator =(const Box &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;

    return *this;
}

Geom::Frame Geom::Box::getFrame()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
{
	return Frame(this->Center, this->Axis[0], this->Axis[1], this->Axis[2]);
}

void Geom::Box::setFrame( Frame f )
{
	this->Center = f.c;
	this->Axis[0] = f.r;
	this->Axis[1] = f.s;
	this->Axis[2] = f.t;
}

Vector3 Geom::Box::getCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

Vector3 Geom::Box::getPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Extent[i] * Axis[i];

	return pos;
}

SurfaceMesh::Vector3 Geom::Box::getPosition( int aid, double c )
{
	return Center + c * Extent[aid] * Axis[aid];
}

void Geom::Box::translate( Vector3 t )
{
	this->Center += t;
}

void Geom::Box::scale( double s )
{
	this->Extent *= s;
}

void Geom::Box::scale( Vector3 s )
{
	for (int i = 0; i < 3; i++)	
		this->Extent[i] *= s[i];
}

void Geom::Box::scale( int axisID, double s )
{
	if (axisID >= 0 && axisID < 3)
		this->Extent[axisID] *= s;
}

bool Geom::Box::hasFaceCoplanarWith( Line line )
{
	Vector3 p1 = line.getPoint(0);
	Vector3 p2 = line.getPoint(1);

	foreach(Plane plane, this->getFacePlanes())
	{
		if (plane.contains(line)) return true;
	}

	return false;
}

QVector<Point> Geom::Box::getConnerPoints()
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

QVector<Geom::Line> Geom::Box::getEdgeLines()
{
	QVector<Line> lines;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 0; i < 12; i++)
	{
		lines.push_back(Line(pnts[ EDGE[i][0] ], pnts[ EDGE[i][1] ]));
	}

	return lines;
}

QVector<Geom::Segment> Geom::Box::getEdgeSegments()
{
	QVector<Segment> edges;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 0; i < 12; i++)
	{
		edges.push_back(Segment(pnts[ EDGE[i][0] ], pnts[ EDGE[i][1] ]));
	}

	return edges;
}


QVector<Geom::Segment> Geom::Box::getEdgeSegments( int aid )
{
	QVector<Segment> edges;

	QVector<Point> pnts = this->getConnerPoints();
	for (int i = 4 * aid; i < 4 * aid + 4; i++)
	{
		edges.push_back(Segment(pnts[EDGE[i][0]], pnts[EDGE[i][1]]));
	}

	return edges;
}


QVector< QVector<Point> > Geom::Box::getFacePoints()
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

QVector<Geom::Plane> Geom::Box::getFacePlanes()
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


QVector<Geom::Rectangle> Geom::Box::getFaceRectangles()
{
	QVector<Rectangle> rects;

	foreach( QVector<Point> conners, this->getFacePoints() )
		rects.push_back(Rectangle(conners));

	return rects;
}


QVector<Geom::Segment> Geom::Box::getEdgeIncidentOnPoint(Point &p)
{
	QVector<Segment> edges;
	
	foreach(Segment s, this->getEdgeSegments()){
		if(s.P0 == p || s.P1 == p)
			edges.push_back(s);
	}

	return edges;
}

QVector<Geom::Rectangle> Geom::Box::getFaceIncidentOnPoint(Point &p)
{
	QVector<Rectangle> rects;

	foreach(Rectangle r, this->getFaceRectangles()){
		foreach(Point cp, r.getConners()){
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
	double gridV = this->volume() / N;
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

double Geom::Box::volume()
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


int Geom::Box::getClosestAxisId( Vector3 a )
{
	double maxDot = minDouble();
	int aid = 0;
	for (int i = 0; i < 3; i++)
	{
		double dotProd = fabs(dot(Axis[i], a));
		if (dotProd > maxDot)
		{
			maxDot = dotProd;
			aid = i;
		}
	}

	return aid;
}


double Geom::Box::calcFrontierWidth( int fid, const QVector<Vector3>& pnts, bool two_side /*= false*/ )
{
	int axisID = fid / 3;
	bool isNeg = fid % 3;

	double maxWidth = -maxDouble();
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

SurfaceMesh::Vector3 Geom::Box::getFaceCenter( int aid, bool positive )
{
	return getFaceCenter(getFaceId(aid, positive));
}


int Geom::Box::getFaceId( int aid, bool positive )
{
	return positive ? (3 * aid) : (3 * aid + 1);
}


void Geom::Box::draw( QColor color )
{
	PolygonSoup ps;
	foreach(QVector<Point> f, getFacePoints()) 
		ps.addPoly(f, color);

	ps.drawQuads(true);
}

void Geom::Box::drawWireframe( double width /*= 2.0*/, QColor color /*= Qt::white*/ )
{
	PolygonSoup ps;
	foreach(QVector<Point> f, getFacePoints()) 
		ps.addPoly(f, color);

	ps.drawWireframes(width, color);
}

Geom::Segment Geom::Box::getSkeleton( int aid )
{
	int fid0 = getFaceId(aid, false);
	int fid1 = getFaceId(aid, true);

	Vector3 fc0 = getFaceCenter(fid0);
	Vector3 fc1 = getFaceCenter(fid1);

	return Segment(fc0, fc1);
}

Geom::Rectangle Geom::Box::getPatch( int aid, double c )
{
	QVector<Vector3> conners;
	foreach(Geom::Segment edge, getEdgeSegments(aid))
	{
		conners.push_back(edge.getPosition(c));
	}

	return Geom::Rectangle(conners);
}

bool Geom::Box::split( int aid, double cp, Box& box1, Box& box2 )
{
	if (!inRange(cp, -1, 1)) return false;

	// cut point on skeleton
	Vector3 cutPoint = getPosition(aid, cp);

	// positive side box
	box1 = *this;
	Vector3 fc1 = getFaceCenter(aid, true);
	box1.Center = (cutPoint + fc1) / 2;
	box1.Extent[aid] *= (1-cp) / 2;

	// negative side box
	box2 = *this;
	Vector3 fc2 = getFaceCenter(aid, false);
	box2.Center = (cutPoint + fc2) / 2;
	box2.Extent[aid] *= (cp+1) / 2;

	return true;
}

void Geom::Box::write( XmlWriter& xw )
{
	xw.writeOpenTag("box");
	{
		xw.writeTaggedString("c", qStr(Center));
		xw.writeTaggedString("x", qStr(Axis[0]));
		xw.writeTaggedString("y", qStr(Axis[1]));
		xw.writeTaggedString("z", qStr(Axis[2]));
		xw.writeTaggedString("e", qStr(Extent));
	}
	xw.writeCloseTag("box");
}

void Geom::Box::read( QDomNode& node )
{
	QString c = node.firstChildElement("c").text();
	QString x = node.firstChildElement("x").text();
	QString y = node.firstChildElement("y").text();
	QString z = node.firstChildElement("z").text();
	QString e = node.firstChildElement("e").text();

	Center = toVector3(c);
	Axis.clear();
	Axis << toVector3(x) << toVector3(y) << toVector3(z);
	Extent = toVector3(e);
}

double Geom::Box::getExtent( int aid )
{
	return Extent[aid];
}

double Geom::Box::getExtent( Vector3 v )
{
	return Extent[getAxisId(v)];
}

void Geom::Box::scaleAsPart(double f, Vector3 c)
{
	QVector<Point> conners = getConnerPoints();
	Vector3 centre = Vector3(0.0,0.0,0.0);
	foreach(Point p, conners){
		p -= c;
		p /= f;
		p += c;
		centre += p;
	}
	this->Center = centre/8;
	this->Extent /= f;
}

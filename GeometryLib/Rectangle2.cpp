#include "Rectangle2.h"


//   1	 _______e2______  0
//		|				|
//		|		Y		|
//		|		|		|
//	e1	|		|___ X	|e0
//		|				|
//		|				|
//	   2|_______________|3
//				e3

int Geom::Rectangle2::EDGE[4][2] = {
	3, 0,
	2, 1,
	1, 0,
	2, 3
};

Geom::Rectangle2::Rectangle2()
{
	Axis.resize(2);
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

	Vector2 e0 = conners[0] - conners[1];
	Vector2 e1 = conners[0] - conners[3];

	// axis: be careful if the rect is degenerated
	if (e0.norm() == 0)
		Axis.push_back(Vector2(0, 0));
	else
		Axis.push_back(e0.normalized());

	if (e1.norm() == 0)
		Axis.push_back(Vector2(0, 0));
	else
		Axis.push_back(e1.normalized());

	Extent = Vector2(e0.norm(), e1.norm()) / 2;
}


QVector<Vector2> Geom::Rectangle2::getConners()
{
	Vector2 extAxis0 = Axis[0] * Extent[0];
	Vector2 extAxis1 = Axis[1] * Extent[1];

	return QVector<Vector2>()
		<< Center + extAxis0 + extAxis1
		<< Center - extAxis0 + extAxis1
		<< Center - extAxis0 - extAxis1
		<< Center + extAxis0 - extAxis1;
}


QStringList Geom::Rectangle2::toStrList()
{
	return QStringList() << "Rectangle2: "
		<< "Center = " + qStr(Center)
		<< "Axis[0] = " + qStr(Axis[0])
		<< "Axis[1] = " + qStr(Axis[1])
		<< "Extent = " + qStr(Extent);
}

Vector2 Geom::Rectangle2::getCoordinates( Vector2& p )
{
	Vector2 v = p - Center;
	double x = dot(v, Axis[0]) / Extent[0];
	double y = dot(v, Axis[1]) / Extent[1];

	return Vector2(x, y);
}

Vector2 Geom::Rectangle2::getPosition( Vector2& coord )
{
	Vector2 pos = Center;
	for (int i = 0; i < 2; i++)
		pos += coord[i]* Extent[i] * Axis[i];

	return pos;
}

SurfaceMesh::Vector2 Geom::Rectangle2::getEdgeCenter(int aid, bool positive)
{
	Vector2 ec = Center;
	Vector2 v = Extent[aid] * Axis[aid];
	if (positive) ec += v;
	else ec -= v;

	return ec;
}

Geom::Segment2 Geom::Rectangle2::getSkeleton(int aid)
{
	Vector2 p0 = getEdgeCenter(aid, true);
	Vector2 p1 = getEdgeCenter(aid, false);
	return Segment2(p0, p1);
}


QVector<Geom::Segment2> Geom::Rectangle2::getEdgeSegments()
{
	QVector<Segment2> edges;

	QVector<Vector2> Conners = getConners();
	for (int i = 0; i < 4; i++)
		edges.push_back(Segment2(Conners[ EDGE[i][0] ], Conners[ EDGE[i][1] ]));

	return edges;
}

// positive thr expands the rectangle while negative thr shrinks the rectangle
bool Geom::Rectangle2::contains( Vector2& p, double thr /*= ZERO_TOLERANCE_LOW*/ )
{
	Vector2 coord = getCoordinates(p);
	double threshold = 1 + thr;

	return (fabs(coord[0]) < threshold) 
		&& (fabs(coord[1]) < threshold);
}

bool Geom::Rectangle2::containsAll(QVector<Vector2>& pnts, double thr /*= ZERO_TOLERANCE_LOW*/ )
{
	bool ctn_all = true;
	foreach(Vector2 p, pnts){
		if (!contains(p, thr)){	ctn_all = false; break;}
	}
	return ctn_all;
}


bool Geom::Rectangle2::containsAny(QVector<Vector2>& pnts, double thr /*= ZERO_TOLERANCE_LOW*/)
{
	bool ctn_any = false;
	for(auto p : pnts){
		if (contains(p, thr)) {ctn_any = true; break;}
	}
	return ctn_any;
}



int Geom::Rectangle2::getAxisId( Vector2& v )
{
	double XdV = fabs(dot(Axis[0], v));
	double YdV = fabs(dot(Axis[1], v));
	return (XdV >= YdV) ? 0 : 1;
}

int Geom::Rectangle2::getPerpAxisId( Vector2& v )
{
	double XdV = fabs(dot(Axis[0], v));
	double YdV = fabs(dot(Axis[1], v));
	return (XdV < YdV) ? 0 : 1;
}

double Geom::Rectangle2::getExtent( Vector2& v )
{
	return Extent[getAxisId(v)];
}

double Geom::Rectangle2::getPerpExtent( Vector2& v )
{
	return Extent[getPerpAxisId(v)];
}

double Geom::Rectangle2::area()
{
	return 4 * Extent[0] * Extent[1];
}


// base       this rect
//		 ______________
//	|	|				|  
//	|	|		Y		|
//	|	|		|		|
//	|	|		|___ X	| 
//	|	|				|
//	|	|				|
//	|	|______________ |
void Geom::Rectangle2::shrinkToExclude( QVector<Vector2>& pnts, Segment2& base )
{
	// try different methods
	QVector<Rectangle2> rects(3, *this);
	rects[0].shrinkFront(pnts, base); 
	rects[1].shrinkLeftRight(pnts, base); 
	rects[2].shrinkFrontLeftRight(pnts, base);

	// choose the one with max area
	double maxArea = minDouble();
	int maxId = 0;
	for (int i = 0; i < rects.size(); i++){
		double a = rects[i].area();
		if (a > maxArea){ maxArea = a; maxId = i; }
	}
	*this = rects[maxId];
}

void Geom::Rectangle2::shrinkFront( QVector<Vector2>& pnts, Segment2& base )
{
	int yId = getAxisId(base.Direction);
	int xId = (yId + 1) % 2;
	bool xflip = false;
	Vector2 c = getCoordinates(base.Center);
	if (c[xId] > 0) xflip = true;

	double xlow = -1, xhigh = 1;
	foreach (Vector2 p, pnts)
	{
		Vector2 coord = getCoordinates(p);
		double x = coord[xId];	if(xflip) x *= -1;

		if (inRange(x, xlow, xhigh)) xhigh = x;
	}

	// shrink
	Vector2 center_coord(0, 0);
	center_coord[xId] = (xlow + xhigh) / 2; 
	if (xflip) center_coord[xId] *= -1;

	Center = getPosition(center_coord);
	Extent[xId] *= (xhigh - xlow) / 2;
}

void Geom::Rectangle2::shrinkLeftRight( QVector<Vector2>& pnts, Segment2& base )
{
	int yId = getAxisId(base.Direction);

	double ylow = -1, yhigh = 1;
	foreach (Vector2 p, pnts)
	{
		Vector2 coord = getCoordinates(p);
		double y = coord[yId];

		if (inRange(y, ylow, yhigh))
		{
			double y_ylow = y - ylow, yhigh_y = yhigh - y;
			if (y_ylow <= yhigh_y)	ylow = y;
			else					yhigh = y;
		}
	}

	// shrink
	Vector2 center_coord(0, 0);
	center_coord[yId] = (ylow + yhigh) / 2;

	Center = getPosition(center_coord);
	Extent[yId] *= (yhigh - ylow) / 2;
}

void Geom::Rectangle2::shrinkFrontLeftRight( QVector<Vector2>& pnts, Segment2& base )
{
	int yId = getAxisId(base.Direction);
	int xId = (yId + 1) % 2;
	double xE = Extent[xId];
	double yE = Extent[yId];
	bool xflip = false;
	Vector2 c = getCoordinates(base.Center);
	if (c[xId] > 0) xflip = true;

	// compute new range along two directions
	double xlow = -1, xhigh = 1, ylow = -1, yhigh = 1;
	foreach (Vector2 p, pnts)
	{
		Vector2 coord = getCoordinates(p);
		double x = coord[xId];	if(xflip) x *= -1;
		double y = coord[yId];
		double xCost = 0, yCost = 0;

		// x direction
		double new_xhigh = xhigh;
		if (inRange(x, xlow, xhigh))
		{
			xCost = (xhigh - x) * yE; 
			new_xhigh = x;
		}

		// y direction
		double new_ylow = ylow, new_yhigh = yhigh;
		if (inRange(y, ylow, yhigh))
		{
			double y_ylow = y - ylow, yhigh_y = yhigh - y;
			if (y_ylow <= yhigh_y){
				yCost = y_ylow * xE; new_ylow = y;
			}else{
				yCost = yhigh_y * xE; new_yhigh = y;
			}
		}

		// shrink range with lower cost
		if (xCost < yCost){
			xhigh = new_xhigh;
		}else{
			ylow = new_ylow; yhigh = new_yhigh;
		}
	}

	// shrink
	Vector2 center_coord(0, 0);
	center_coord[xId] = (xlow + xhigh) / 2; if (xflip) center_coord[xId] *= -1;
	center_coord[yId] = (ylow + yhigh) / 2;

	Center = getPosition(center_coord);
	Extent[xId] *= (xhigh - xlow) / 2;
	Extent[yId] *= (yhigh - ylow) / 2;
}


void Geom::Rectangle2::cropByAxisAlignedRectangle( Rectangle2& cropper )
{
	for (int i = 0; i < 2; i++)
	{
		// skeleton along axis[i]
		Segment2 sklt = getSkeleton(i);
		int caid = cropper.getAxisId(Axis[i]);
		Segment2 c_sklt = cropper.getSkeleton(caid);

		// point to same direction
		if (dot(sklt.Direction, c_sklt.Direction) < 0)
			c_sklt.flip();

		// projection
		double t0 = c_sklt.getProjCoordinates(sklt.P0);
		double t1 = c_sklt.getProjCoordinates(sklt.P1);
		double t0_crop = Max(-1, t0);
		double t1_crop = Min(1, t1);

		// no overlapping along this direction
		if (t0_crop >= t1_crop) 
		{
			Extent *= 0;
			return;
		}
		// crop along this direction
		else
		{
			// move center
			Vector2 c = c_sklt.getPosition((t0+t1)/2);
			Vector2 c_crop = c_sklt.getPosition((t0_crop+t1_crop)/2);
			Center += c_crop - c;

			// change extent
			Vector2 p0 = c_sklt.getPosition(t0_crop);
			Vector2 p1 = c_sklt.getPosition(t1_crop);
			Extent[i] = (p0 - p1).norm()/2;
		}
	}
}


QVector<Vector2> Geom::Rectangle2::getEdgeSamples(int N)
{
	QVector<Vector2> samples;
	foreach(Segment2 edge, getEdgeSegments())
		samples += edge.getUniformSamples(N);

	return samples;
}

QVector<Vector2> Geom::Rectangle2::getGridSamples( double w)
{
	QVector<Vector2> samples;

	int resX = (int)ceil(2 * Extent[0] / w);
	int resY = (int)ceil(2 * Extent[1] / w);

	for (int i = 0; i <= resX; i++){
		for (int j = 0; j <= resY; j++)
			{
				double ci = -1 + i * w;
				double cj = -1 + j * w;
				samples.push_back(this->getPosition(Vector2(ci, cj)));
			}
	}

	return samples;
}


void Geom::Rectangle2::expandToTouch(QVector<Vector2>& pnts, double thr)
{
	// results from various methods
	QVector<Rectangle2> rects;

	// method-1: x, y
	Rectangle2 rect_xy = *this;
	rect_xy.expandAlongSingleAxisToTouch(pnts, 0, thr);
	rect_xy.expandAlongSingleAxisToTouch(pnts, 1, thr);
	rects << rect_xy;

	// method-2: y, x
	Rectangle2 rect_yx = *this;
	rect_yx.expandAlongSingleAxisToTouch(pnts, 1, thr);
	rect_yx.expandAlongSingleAxisToTouch(pnts, 0, thr);
	rects << rect_yx;

	// choose the one with max area
	double maxArea = minDouble();
	int maxId = 0;
	for (int i = 0; i < rects.size(); i++){
		double a = rects[i].area();
		if (a > maxArea){maxArea = a; maxId = i;}
	}
	*this = rects[maxId];
}

void Geom::Rectangle2::expandAlongSingleAxisToTouch(QVector<Vector2>& pnts, int aid, double thr /*= ZERO_TOLERANCE_LOW*/)
{
	// axes
	int aid_free = aid;
	int aid_fixed = 1 - aid;

	// bounds on two sides
	double left = -maxDouble();
	double right = maxDouble();

	// threshold along fixed axis
	double threshold = 1 + thr;

	// update bounds by each point
	for (auto p : pnts)
	{
		Vector2 pc = getCoordinates(p);
		double u = pc[aid_free];
		double v = pc[aid_fixed];

		// check the extent along the fixed axis
		if (fabs(v) < threshold)
		{
			// tightest bound on left
			if (u < 0 && u > left) left = u;
			// tightest bound on right
			if (u > 0 && u < right) right = u;
		}
	}

	// update this rec
	Vector2 center_coord(0, 0);
	center_coord[aid_free] = (left + right) / 2;
	Center = getPosition(center_coord);

	double width = right - left;
	Extent[aid_free] *= width / 2;}

Geom::Rectangle2 Geom::Rectangle2::computeAABB(QVector<Vector2> &pnts)
{
	// compute extent along x and y
	double minX = maxDouble();
	double maxX = -maxDouble();
	double minY = maxDouble();
	double maxY = -maxDouble();
	for (auto p : pnts)
	{
		if (p.x() < minX) minX = p.x();
		if (p.x() > maxX) maxX = p.x();

		if (p.y() < minY) minY = p.y();
		if (p.y() > maxY) maxY = p.y();
	}

	// create rect
	QVector<Vector2> conners;
	conners << Vector2(minX, minY) << Vector2(maxX, minY) << Vector2(maxX, maxY) << Vector2(minX, maxY);
	return Rectangle2(conners);
}

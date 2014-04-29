#include "Rectangle2.h"
#include "Numeric.h"

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

void Geom::Rectangle2::copyFrom( Rectangle2& other )
{
	Center = other.Center;
	Axis = other.Axis;
	Extent = other.Extent;
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

QVector<Geom::Segment2> Geom::Rectangle2::getEdgeSegments()
{
	QVector<Segment2> edges;

	QVector<Vector2> Conners = getConners();
	for (int i = 0; i < 4; i++)
		edges.push_back(Segment2(Conners[ EDGE[i][0] ], Conners[ EDGE[i][1] ]));

	return edges;
}

QVector<Vector2> Geom::Rectangle2::getEdgeSamples( int N )
{
	QVector<Vector2> samples;
	foreach (Segment2 edge, getEdgeSegments())
		samples += edge.getUniformSamples(N);

	return samples;
}

bool Geom::Rectangle2::contains( Vector2& p )
{
	Vector2 coord = getCoordinates(p);
	double threshold = 1 + ZERO_TOLERANCE_LOW;

	return (fabs(coord[0]) < threshold) 
		&& (fabs(coord[1]) < threshold);
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


//    ______________
//	||				|  
//	||		Y		|
//	||		|		|
//	||		|___ X	| 
//	||				|
//	||				|
//	||______________|
void Geom::Rectangle2::shrinkToAvoidPoints( QVector<Vector2>& pnts, Segment2& base )
{
	QVector<Rectangle2> rects;
	rects << shrinkFront(pnts, base) << shrinkLeftRight(pnts, base) << shrinkFrontLeftRight(pnts, base);

	double maxArea = minDouble();
	int maxId = 0;
	for (int i = 0; i < 3; i++)
	{
		double a = rects[i].area();
		if (a > maxArea)
		{
			maxArea = a;
			maxId = i;
		}
	}

	copyFrom(rects[maxId]);
}

Geom::Rectangle2 Geom::Rectangle2::shrinkFront( QVector<Vector2>& pnts, Segment2& base )
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

	Rectangle2 shrunk_rect = *this;
	shrunk_rect.Center = getPosition(center_coord);
	shrunk_rect.Extent[xId] *= (xhigh - xlow) / 2;

	return shrunk_rect;
}

Geom::Rectangle2 Geom::Rectangle2::shrinkLeftRight( QVector<Vector2>& pnts, Segment2& base )
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

	Rectangle2 shrunk_rect = *this;
	shrunk_rect.Center = getPosition(center_coord);
	shrunk_rect.Extent[yId] *= (yhigh - ylow) / 2;

	return shrunk_rect;
}

Geom::Rectangle2 Geom::Rectangle2::shrinkFrontLeftRight( QVector<Vector2>& pnts, Segment2& base )
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

	Rectangle2 shrunk_rect = *this;
	shrunk_rect.Center = getPosition(center_coord);
	shrunk_rect.Extent[xId] *= (xhigh - xlow) / 2;
	shrunk_rect.Extent[yId] *= (yhigh - ylow) / 2;

	return shrunk_rect;
}

SurfaceMesh::Vector2 Geom::Rectangle2::getEdgeCenter( int aid, bool positive )
{
	Vector2 ec = Center;
	Vector2 v = Extent[aid] * Axis[aid];
	if (positive) ec += v;
	else ec -= v;

	return ec;
}

Geom::Segment2 Geom::Rectangle2::getSkeleton( int aid )
{
	Vector2 p0 = getEdgeCenter(aid, true);
	Vector2 p1 = getEdgeCenter(aid, false);
	return Segment2(p0, p1);
}

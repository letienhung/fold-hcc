#pragma once

#include "Frame.h"
#include "Plane.h"
#include "Line.h"
#include "Segment.h"
#include "Rectangle.h"

namespace Geom{

class Box 
{
public:
	// core data
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// con(de)structor
	Box(){Axis.resize(3);}
	~Box(){}
	Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);

	// assignment
	Box &operator =(const Box &);

	// frame
	Frame	getFrame();
	void	setFrame(Frame f);
	int		getFaceId(Vector3 n);
	int		getFaceId(int aid, bool positive);
	int		getAxisId(Vector3 a);
	int		minAxisId();
	int		maxAxisId();

	// coordinates
	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);

	// transform
	void translate(Vector3 t);
	void scale(double s);
	void scale(Vector3 s);
	void scale(int axisID, double s);
	Box  scaled(double s);

	// geometry
	static int NB_FACES;
	static int NB_EDGES;
	static int NB_VERTICES;
	static int EDGE[12][2];
	static int QUAD_FACE[6][4];
	static int TRI_FACE[12][3];

	Vector3 getFaceCenter(int fid);

	QVector<Point>				getConnerPoints();
	QVector<Line>				getEdgeLines();
	QVector<Segment>			getEdgeSegments();
	QVector<Segment>			getEdgeSegmentsAlongAxis(int aid);
	QVector< QVector<Point> >	getFacePoints();
	QVector<Plane>				getFacePlanes();
	QVector<Rectangle>			getFaceRectangles();

	QVector<Segment>            getEdgeIncidentOnPoint(Point &p);
	QVector<Rectangle>          getFaceIncidentOnPoint(Point &p);

	// sampling
	double	getVolume();
	QVector<Vector3>	getGridSamples(int N);

	// tags
	QVector<bool> edgeTags;

	// relation with  other objects
	bool hasFaceCoplanarWith(Line line);
	bool contains(Vector3 p);

	// frontier
	double calcFrontierWidth(int fid, const QVector<Vector3>& pnts, bool two_side = false);
	Vector4 calcFrontierWidth(Vector3 hX, Vector3 hZ, const QVector<Vector3>& pnts);

	// type
	enum TYPE{ROD, PATCH, BRICK};
	int getType(double threshold);

};

}
#pragma once

#include "Frame.h"
#include "Plane.h"
#include "Line.h"
#include "Segment.h"
#include "Rectangle.h"

#include "XmlWriter.h"
#include <QDomNode>

namespace Geom{

class Box 
{
public:
	// core data
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// constructor
	Box();
	Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);
	Box(const Frame& f, const Vector3& ext);
	Box(const Rectangle& rect, const Vector3& n, const double& height);

	// helper
	void makeRightHanded();
	void normalizeAxis();

	// assignment
	Box &operator =(const Box &);

	// frame
	Frame	getFrame();
	void	setFrame(Frame f);

	// coordinates
	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);
	Vector3 getPosition(int aid, double c);

	// transform 
	void translate(Vector3 t);
	void scale(double s);
	void scale(Vector3 s);
	void scale(int axisID, double s);
	Box  scaled(double s);
	void scaleAsPart(double f, Vector3 c);

	// split
	bool split(int aid, double cp, Box& box1, Box& box2);

	// geometry
	static int NB_FACES;
	static int NB_EDGES;
	static int NB_VERTICES;
	static int EDGE[12][2];
	static int QUAD_FACE[6][4];
	static int TRI_FACE[12][3];

	double	radius();
	double	volume();
	int		getFaceId(Vector3 n);
	int		getFaceId(int aid, bool positive);
	int		getAxisId(Vector3 a);
	int		getClosestAxisId(Vector3 a);
	int		minAxisId();
	int		maxAxisId();
	double	getExtent(int aid);
	double	getExtent(Vector3 v);
	Vector3 getFaceCenter(int fid);
	Vector3 getFaceCenter(int aid, bool positive);
	Segment getSkeleton(int aid);
	Rectangle getPatch(int aid, double c);

	QVector<Point>				getConnerPoints();
	QVector<Line>				getEdgeLines();
	QVector<Segment>			getEdgeSegments();
	QVector<Segment>			getEdgeSegments(int aid);
	QVector< QVector<Point> >	getFacePoints();
	QVector<Plane>				getFacePlanes();
	QVector<Rectangle>			getFaceRectangles();
	QVector<Segment>            getEdgeIncidentOnPoint(Point &p);
	QVector<Rectangle>          getFaceIncidentOnPoint(Point &p);

	// sampling
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

	// draw
	void draw(QColor color = Qt::white);
	void drawWireframe(double width = 2.0, QColor color = Qt::white);

	// I/O
	void write( XmlWriter& xw);
	void read(QDomNode& node);
};

}
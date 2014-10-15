#pragma once
#include "UtilityGlobal.h"
#include "Segment.h"
#include "Box.h"

class Scaffold;

class VisualDebugger
{
public:
    VisualDebugger();

	// visualize
	void draw();

	// clear
	void clearAll();

	// setters
	void addPoint(Vector3& pnt, Qt::GlobalColor color = Qt::black);
	void addPoints(QVector<Vector3>& pnts, Qt::GlobalColor color = Qt::black);
	void addSegment(Geom::Segment& seg, Qt::GlobalColor color = Qt::black);
	void addSegments(QVector<Geom::Segment>& segs, Qt::GlobalColor color = Qt::black);
	void addRectangle(Geom::Rectangle& rect, Qt::GlobalColor color = Qt::black);
	void addRectangles(QVector<Geom::Rectangle>& rects, Qt::GlobalColor color = Qt::black);
	void addBox(Geom::Box& box, Qt::GlobalColor color = Qt::black);
	void addBoxes(QVector<Geom::Box>& boxes, Qt::GlobalColor color = Qt::black);
	void addPlane(Geom::Plane& plane, Qt::GlobalColor color = Qt::black);
	void addPlanes(QVector<Geom::Plane>& planes, Qt::GlobalColor color = Qt::black);

	void addScaffold(Scaffold* scaff);

public:
	QMap< Qt::GlobalColor, QVector<Vector3> > colorPoints;
	QMap< Qt::GlobalColor, QVector<Geom::Segment> > colorSegments;
	QMap< Qt::GlobalColor, QVector<Geom::Rectangle> > colorRectangles;
	QMap< Qt::GlobalColor, QVector<Geom::Box> > colorBoxes;
	QMap< Qt::GlobalColor, QVector<Geom::Plane> > colorPlanes;

	QVector<Scaffold*> scaffolds;
};

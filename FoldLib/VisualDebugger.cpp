#include "VisualDebugger.h"
#include "CustomDrawObjects.h"
#include "Scaffold.h"

VisualDebugger::VisualDebugger()
{
}

void VisualDebugger::clearAll()
{
	colorPoints.clear();
	colorSegments.clear();
	colorRectangles.clear();
	colorBoxes.clear();
	colorPlanes.clear();

	for (Scaffold* scaff : scaffolds) delete scaff;
	scaffolds.clear();
}

void VisualDebugger::draw()
{
	// color points
	for (Qt::GlobalColor color : colorPoints.keys()){
		PointSoup ps;
		for (Vector3 p : colorPoints[color])
			ps.addPoint(p, color);
		ps.draw();
	}

	// color segments
	for (Qt::GlobalColor color : colorSegments.keys()){
		for (Geom::Segment sg : colorSegments[color])
			sg.draw(3.0, color);
	}

	// color rectangles
	for (Qt::GlobalColor color : colorRectangles.keys()){
		for (Geom::Rectangle rect : colorRectangles[color])
			rect.drawEdges(2.0, color);
	}

	// color boxes
	for (Qt::GlobalColor color : colorBoxes.keys()){
		for (Geom::Box box : colorBoxes[color])
			box.drawWireframe(2.0, color);
	}

	// color planes
	for (Qt::GlobalColor color : colorPlanes.keys()){
		for (Geom::Plane plane : colorPlanes[color]){
			PlaneSoup ps(2.0, color);
			ps.addPlane(plane.Constant, plane.Normal);
			ps.draw();
		}
	}

	// scaffold
	for (Scaffold* ds : scaffolds){
		if (ds)	{
			ds->draw();
		}
	}
}

void VisualDebugger::addPoint(Vector3& pnt, Qt::GlobalColor color /*= Qt::black*/)
{
	colorPoints[color] << pnt;
}

void VisualDebugger::addPoints(QVector<Vector3>& pnts, Qt::GlobalColor color /*= Qt::black*/)
{
	colorPoints[color] << pnts;
}

void VisualDebugger::addSegment(Geom::Segment& seg, Qt::GlobalColor color /*= Qt::black*/)
{
	colorSegments[color] << seg;
}

void VisualDebugger::addSegments(QVector<Geom::Segment>& segs, Qt::GlobalColor color /*= Qt::black*/)
{
	colorSegments[color] << segs;
}

void VisualDebugger::addRectangle(Geom::Rectangle& rect, Qt::GlobalColor color /*= Qt::black*/)
{
	colorRectangles[color] << rect;
}

void VisualDebugger::addRectangles(QVector<Geom::Rectangle>& rects, Qt::GlobalColor color /*= Qt::black*/)
{
	colorRectangles[color] << rects;
}

void VisualDebugger::addBox(Geom::Box& box, Qt::GlobalColor color /*= Qt::black*/)
{
	colorBoxes[color] << box;
}

void VisualDebugger::addBoxes(QVector<Geom::Box>& boxes, Qt::GlobalColor color /*= Qt::black*/)
{
	colorBoxes[color] << boxes;
}

void VisualDebugger::addPlane(Geom::Plane& plane, Qt::GlobalColor color /*= Qt::black*/)
{
	colorPlanes[color] << plane;
}

void VisualDebugger::addPlanes(QVector<Geom::Plane>& planes, Qt::GlobalColor color /*= Qt::black*/)
{
	colorPlanes[color] << planes;
}

void VisualDebugger::addScaffold(Scaffold* scaff)
{
	scaffolds << (Scaffold*)scaff->clone();
}

#include "VisualDebugger.h"
#include "CustomDrawObjects.h"
#include "Scaffold.h"

VisualDebugger::VisualDebugger()
{
}

void VisualDebugger::clearDebug()
{
	debugPntsR.clear();	debugPntsG.clear();	debugPntsB.clear();
	debugSegsR.clear();	debugSegsG.clear();	debugSegsB.clear();
	debugRectsR.clear();debugRectsG.clear();debugRectsB.clear();
	debugBoxes.clear();
	debugPlanes.clear();
	debugScaffs.clear();
}

void VisualDebugger::draw()
{
	// debug points
	PointSoup psR, psG, psB;
	for (Vector3 p : debugPntsR) psR.addPoint(p, Qt::red); psR.draw();
	for (Vector3 p : debugPntsG) psG.addPoint(p, Qt::green); psG.draw();
	for (Vector3 p : debugPntsB) psB.addPoint(p, Qt::blue); psB.draw();

	// debug segments
	for (Geom::Segment seg : debugSegsR) seg.draw(3.0, Qt::red);
	for (Geom::Segment seg : debugSegsG) seg.draw(3.0, Qt::green);
	for (Geom::Segment seg : debugSegsB) seg.draw(3.0, Qt::blue);

	// debug rectangles
	for (Geom::Rectangle rect : debugRectsR) rect.drawEdges(2.0, Qt::red);
	for (Geom::Rectangle rect : debugRectsG) rect.drawEdges(2.0, Qt::green);
	for (Geom::Rectangle rect : debugRectsB) rect.drawEdges(2.0, Qt::blue);

	// debug boxes
	QColor color = Qt::green; color.setAlphaF(1.0);
	for (Geom::Box box : debugBoxes) box.drawWireframe(2.0, color);

	// debug planes
	for (Geom::Plane p : debugPlanes) p.draw();

	// scaffold
	for (Scaffold* ds : debugScaffs){
		if (ds)	{
			ds->showCuboids(false);
			ds->showScaffold(true);
			ds->showMeshes(false);
			ds->draw();
		}
	}
}

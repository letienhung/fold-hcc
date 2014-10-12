#pragma once
#include "UtilityGlobal.h"
#include "Segment.h"
#include "Box.h"

class Scaffold;

class VisualDebugger
{
public:
    VisualDebugger();

	void clearDebug();
	void draw();

public:
	QVector<Vector3> debugPntsR, debugPntsG, debugPntsB;
	QVector<Geom::Segment> debugSegsR, debugSegsG, debugSegsB;
	QVector<Geom::Rectangle> debugRectsR, debugRectsG, debugRectsB;

	QVector<Geom::Box> debugBoxes;
	QVector<Geom::Plane> debugPlanes;

	QVector<Scaffold*> debugScaffs;
};

#pragma once

#include "Segment.h"
#include "FdGraph.h"
#include "PatchNode.h"
#include "SectorCylinder.h"
#include "DependGraph.h"

class PizzaChain : public FdGraph
{
public:
    PizzaChain(FdNode* part, PatchNode* panel, QString id);

	Geom::SectorCylinder getFoldingVolume(FoldingNode* fn);

public:
	FdNode* mPart;
	PatchNode* mPanel;

	QVector<Geom::Segment> hinges;
	Geom::Segment r1; // perp seg on part
	QVector<Vector3> v2s; // perp direction on panel to the right
};

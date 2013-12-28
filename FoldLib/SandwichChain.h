#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Rectangle2.h"
#include "DependGraph.h"

class SandwichChain : public FdGraph
{
public:
    SandwichChain(FdNode* part, PatchNode* panel1, PatchNode* panel2);

	Geom::Rectangle2 getFoldingArea(FoldingNode* fn);

public:
	FdNode		*mPart;
	PatchNode	*mPanel1, *mPanel2;

	QVector<Geom::Segment>  hinges;
	Geom::Segment			r1;		// perp seg on part
	QVector<Vector3>		v2s;	// perp direction on panel to the right
	double					nbFold;	// number of folds, default is 2
};


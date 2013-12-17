#pragma once

#include "FdNode.h"
#include "Link.h"
#include "Segment.h"

class FdLink : public Structure::Link
{
public:
    FdLink(FdNode* n1, FdNode* n2, Geom::Segment& distSeg);
	FdLink(FdLink& other);
	virtual Link* clone();

	virtual void draw();

public:
	Geom::Segment mLink;
};



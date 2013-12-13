#pragma once

#include "FdNode.h"
#include "Link.h"
#include "Segment.h"

class FdLink : public Structure::Link
{
public:
    FdLink(FdNode* n1, FdNode* n2);

	void draw();
	Geom::Segment mSeg;
};



#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(SurfaceMeshModel *m, Geom::Box &b);

	void draw();

private:
	Geom::Segment rod;
};
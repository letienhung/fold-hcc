#pragma once

#include "FdNode.h"

class PatchNode : public FdNode
{
public:
    PatchNode(SurfaceMeshModel *m, Geom::Box &b);
	void draw();

private:
	Geom::Rectangle mPatch;
};


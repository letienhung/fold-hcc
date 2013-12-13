#pragma once

#include "FdNode.h"

class PatchNode : public FdNode
{
public:
    PatchNode(SurfaceMeshModel* m, Geom::Box &b);
	~PatchNode();

public:
	void draw();
	void refit(int method);
	void createPatch();
private:
	Geom::Rectangle mPatch;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


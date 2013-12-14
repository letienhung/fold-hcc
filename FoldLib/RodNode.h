#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(SurfaceMeshModel* m, Geom::Box &b);
	~RodNode();

public:
	void draw();
	void createRod();
	void refit(int method);

private:
	Geom::Segment rod;
};
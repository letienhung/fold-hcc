#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(MeshPtr m, Geom::Box &b);
	~RodNode();

public:
	void draw();
	void createRod();
	void refit(int method);

private:
	Geom::Segment rod;
};
#pragma once

#include "FdNode.h"

class PatchNode : public FdNode
{
public:
    PatchNode(MeshPtr m, Geom::Box &b);
	PatchNode(PatchNode &other);
	~PatchNode();
	virtual Node* clone();

public:
	void draw();
	void refit(int method);
	void createPatch();

	bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Rectangle mPatch;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


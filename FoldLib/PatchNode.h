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
	void createPatch();

	// virtual functions
	void refit(int method);
	bool isPerpTo(Vector3 v, double dotThreshold);
	FdNode* split(Geom::Plane& plane);
	void draw();

public:
	Geom::Rectangle mPatch;
	QColor mPatchColor;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


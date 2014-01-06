#pragma once

#include "FdNode.h"

class PatchNode : public FdNode
{
public:
    PatchNode(MeshPtr m, Geom::Box &b);
	PatchNode(PatchNode &other);
	virtual Node* clone();

public:
	void createScaffold();
	bool isPerpTo(Vector3 v, double dotThreshold);
	void drawScaffold();

	Geom::Plane getSurfacePlane(bool positive);

public:
	Geom::Rectangle mPatch;
	QColor mPatchColor;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


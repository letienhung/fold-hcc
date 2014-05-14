#pragma once

#include "FdNode.h"

class RodNode;

class PatchNode : public FdNode
{
public:
    PatchNode(QString id, Geom::Box &b, MeshPtr m);
	PatchNode(QString id, Geom::Box &b, Vector3 v, MeshPtr m);
	PatchNode(PatchNode &other);
	virtual Node* clone();

public:
	// create patch
	void createScaffold(bool useAid);

	// visual
	void drawScaffold();

	// geometry
	double getThickness();
	bool isPerpTo(Vector3 v, double dotThreshold);
	Geom::Plane getSurfacePlane(bool positive);

	// edges
	QVector<RodNode*> getEdgeRodNodes();

public:
	Geom::Rectangle mPatch;
	QColor mPatchColor;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


#pragma once

#include "FdNode.h"

class RodNode;

class PatchNode : public FdNode
{
public:
    PatchNode(QString id, Geom::Box &b, MeshPtr m);
	PatchNode(RodNode* rodNode, Vector3 v);
	PatchNode(PatchNode &other);
	virtual Node* clone();

public:
	// create patch
	void createScaffold(bool useAid);

	// visual
	void drawScaffold();

	// geometry
	double getThickness();
	void setThickness(double thk);
	bool isPerpTo(Vector3 v, double dotThreshold);
	Geom::Plane getSurfacePlane(bool positive);
	void resize(Geom::Rectangle2& newPatch);

	// edges
	QVector<RodNode*> getEdgeRodNodes();

public:
	Geom::Rectangle mPatch;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


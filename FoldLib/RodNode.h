#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(MeshPtr m, Geom::Box &b);
	RodNode(RodNode& other);
	~RodNode();

	virtual Node* clone();

public:
	void draw();
	void createRod();
	void refit(int method);

	bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Segment mRod;
	QColor mRodColor;
};
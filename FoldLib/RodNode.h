#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(MeshPtr m, Geom::Box &b);
	RodNode(RodNode& other);
	~RodNode();

	Node* clone();

public:
	void drawScaffold();
	void createScaffold();

	bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Segment mRod;
	QColor mRodColor;
};
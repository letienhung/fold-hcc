#pragma once

#include "FdNode.h"

class RodNode : public FdNode
{
public:
    RodNode(QString id, Geom::Box &b, MeshPtr m);
	RodNode(RodNode& other);
	~RodNode();

	Node* clone();

public:
	void drawScaffold();
	void createScaffold(bool useAid);

	bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Segment mRod;
	QColor mRodColor;
};
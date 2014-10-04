#pragma once

#include "ScaffNode.h"

class RodNode final : public ScaffNode
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

	void setThickness(double thk);

	// samples
	virtual QVector<Vector3> sampleBoundabyOfScaffold(int n) override;

public:
	Geom::Segment mRod;
	QColor mRodColor;
};
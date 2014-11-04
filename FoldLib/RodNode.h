#pragma once

#include "ScaffNode.h"

class RodNode final : public ScaffNode
{
public:
    RodNode(QString id, Geom::Box &b, MeshPtr m);
	RodNode(RodNode& other);
	virtual ~RodNode();

	virtual	Node* clone() override;

public:
	virtual void drawScaffold() override;
	virtual void createScaffold(bool useAid) override;

	virtual bool isPerpTo(Vector3 v, double dotThreshold) override;

	virtual void setThickness(double thk) override;

	// samples
	virtual QVector<Vector3> sampleBoundabyOfScaffold(int n) override;

public:
	Geom::Segment mRod;
	QColor mRodColor;
};
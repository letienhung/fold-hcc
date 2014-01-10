#pragma once
#include "LayerGraph.h"
#include "SectorCylinder.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> parts, PatchNode* panel, QString id, Geom::Box &bBox);
	~PizzaLayer();

	void buildDependGraph();

	double computeCost(QString fnid);
	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, FdNode* node);
	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, Geom::Rectangle& rect);
	void resolveCollision();

	QVector<Structure::Node*> getKeyFrameNodes( double t );

public: 
	PatchNode* mPanel; 
};
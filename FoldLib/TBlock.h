#pragma once
#include "BlockGraph.h"
#include "SectorCylinder.h"

class TBlock : public BlockGraph
{
public:
    TBlock(QVector<FdNode*> parts, PatchNode* panel, QString id, Geom::Box &bBox);
	~TBlock();

	void foldabilize();
	void buildDependGraph();

	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, FdNode* node);
	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, Geom::Rectangle& rect);
	QVector<Structure::Node*> getKeyFrameNodes( double t );

public: 
	PatchNode* mPanel; 
};
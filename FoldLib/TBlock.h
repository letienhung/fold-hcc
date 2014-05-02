#pragma once
#include "BlockGraph.h"
#include "SectorCylinder.h"

class TBlock : public BlockGraph
{
public:
    TBlock(PatchNode* master, FdNode* slave, QString id);
	~TBlock();

	// fold option
	QVector<FoldOption*> generateFoldOptions();

	void foldabilize();

	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, FdNode* node);
	Vector3 getClosestCoordinates(Geom::SectorCylinder& fVolume, Geom::Rectangle& rect);
	QVector<Structure::Node*> getKeyFrameNodes( double t );

public: 
	PatchNode* mMaster; 
};
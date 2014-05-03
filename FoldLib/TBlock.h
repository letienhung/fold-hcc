#pragma once
#include "BlockGraph.h"
#include "SectorCylinder.h"

class TChain;

class TBlock : public BlockGraph
{
public:
    TBlock(PatchNode* master, FdNode* slave, QString id);
	~TBlock();

	// fold option
	QVector<FoldOption*> generateFoldOptions();

	// foldem
	void foldabilize();

	// results
	QVector<Structure::Node*> getKeyFrameNodes( double t );
public: 
	PatchNode* mMaster; 
};
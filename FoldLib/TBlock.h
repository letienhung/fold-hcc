#pragma once
#include "BlockGraph.h"
#include "SectorCylinder.h"

class TChain;

class TBlock : public BlockGraph
{
public:
    TBlock(PatchNode* master, FdNode* slave, QString id);
	~TBlock();

	// foldem
	QVector<FoldOption*> generateFoldOptions();
	void applyFoldOption(FoldOption* fn);

	// key frame
	FdGraph* getKeyframeScaffold(double t);

	// getters
	int nbTimeUnits();

public: 
	PatchNode* mMaster; 
};
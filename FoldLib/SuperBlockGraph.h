#pragma once

#include "FdGraph.h"
#include "PatchNode.h"

class BlockGraph;
class ShapeSuperKeyframe;

// superBlock is a regular block with masters replaced by their corresponding super masters
// super block is used for computing folding volumes

class SuperBlockGraph : public FdGraph
{
public:
	SuperBlockGraph(BlockGraph* block, ShapeSuperKeyframe* ssKeyframe);

	// folding space
	void computeMinFoldingRegion();
	void computeMaxFoldingRegion();
	void computeAvailFoldingRegion();

	// helpers
	QVector<QString> getInbetweenExternalParts(QString base_mid, QString top_mid);
	QVector<QString> getUnrelatedMasters(QString base_mid, QString top_mid);

	// volume
	double getAvailFoldingVolume();

	// space
	Geom::Box getAFS(QString mid);
	QVector<Geom::Box> getAllAFS();

public:
	// the original block
	BlockGraph* origBlock;

	// the super shape key frame
	ShapeSuperKeyframe* ssKeyframe;

	// super 
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, double> masterHeight;
	QMap<QString, QSet<int> > masterUnderChainsMap;
	QMap<int, QString> chainTopMasterMap;

	// folding regions
	// ***2D rectangles encoded in original base patch
	QMap<QString, Geom::Rectangle2> minFoldingRegion;
	QMap<QString, Geom::Rectangle2> maxFoldingRegion;
	QMap<QString, Geom::Rectangle2> availFoldingRegion;
};

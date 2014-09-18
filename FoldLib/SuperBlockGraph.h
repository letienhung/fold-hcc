#pragma once

#include "FdGraph.h"
#include "PatchNode.h"

class HBlockGraph;
class ShapeSuperKeyframe;

// superBlock is a regular block with masters replaced by their corresponding super masters
// super block is used for computing folding volumes

class SuperBlockGraph : public FdGraph
{
public:
	SuperBlockGraph(HBlockGraph* block, ShapeSuperKeyframe* ssKeyframe);

	// obstacles for each top master
	QMap< QString, QVector<Vector2> > computeObstacles();

private:
	// unrelated masters
	QVector<QString> getUnrelatedExternalMasters(QString base_mid, QString top_mid);

private:
	// the original block
	HBlockGraph* origBlock;

	// the super shape key frame
	ShapeSuperKeyframe* ssKeyframe;

	// super masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
};

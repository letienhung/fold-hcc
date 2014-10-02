#pragma once

#include "Scaffold.h"
#include "PatchNode.h"

class HUnitScaff;
class ShapeSuperKeyframe;

// superBlock is a regular block with masters replaced by their corresponding super masters
// super block is used for computing folding volumes

class SuperUnitScaff : public Scaffold
{
public:
	SuperUnitScaff(HUnitScaff* block, ShapeSuperKeyframe* ssKeyframe);

	// obstacles for each top master
	QMap< QString, QVector<Vector2> > computeObstacles();

private:
	// unrelated masters
	QVector<QString> getUnrelatedExternalMasters(QString base_mid, QString top_mid);

private:
	// the original block
	HUnitScaff* origBlock;

	// the super shape key frame
	ShapeSuperKeyframe* ssKeyframe;

	// super masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
};

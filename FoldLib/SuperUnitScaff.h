#pragma once

#include "Scaffold.h"
#include "PatchNode.h"

class HUnitScaff;
class ShapeSuperKeyframe;

// superBlock is a regular block with masters replaced by their corresponding super masters
// super block is used for computing folding volumes

class SuperUnitScaff final : public Scaffold
{
public:
	SuperUnitScaff(HUnitScaff* block, ShapeSuperKeyframe* ssKeyframe);

	// obstacles for each top master
	QMap< QString, QVector<Vector2> > computeObstacles();

private:
	// the original block
	HUnitScaff* origUnit;

	// the super shape key frame
	ShapeSuperKeyframe* ssKeyframe;

	// super masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
};

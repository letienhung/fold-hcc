#pragma once

#include "Scaffold.h"
#include "PatchNode.h"

class UnitScaff;
class SuperShapeKf;
class HUnitSolution;

// superUnit is a modified regular unit with masters replaced by their corresponding super masters

class SuperUnitScaff final : public Scaffold
{
public:
	SuperUnitScaff(UnitScaff* unit, SuperShapeKf* ssKeyframe);

	// obstacles for each top master
	QMap< QString, QVector<Vector2> > computeObstacles(HUnitSolution* sln);

private:
	// the original block
	UnitScaff* origUnit;

	// the super shape key frame
	SuperShapeKf* ssKeyframe;

	// super masters: super or regular
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
};

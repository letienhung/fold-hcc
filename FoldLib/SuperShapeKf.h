#pragma once

#include "Scaffold.h"

class UnitScaff;

// This class represents a key frame of the shape under certain time
// Different from regular key frame, this class introduce "super master"
// to represent collapsed/merged masters and their mapping relations as well
// The master order constrains are also updated for super masters
// These additional information helps compute various folding volumes

class SuperShapeKf final : public Scaffold
{
public:
	SuperShapeKf(Scaffold* superKeyframe, StringSetMap moc_g);
	// the map between master and super master
	QMap<QString, QString> master2SuperMap;

	// update the master oder constrains with super masters
	StringSetMap mocGreater, mocLess;

	// valid only if all order constraints are met
	bool isValid(Vector3 sqzV);

	// in between parts
	QVector<ScaffNode*> getInbetweenExternalParts(UnitScaff* unit, Vector3 p0, Vector3 p1);

	// unrelated masters
	QVector<ScaffNode*> getUnrelatedExternalMasters(UnitScaff* unit, QString base_mid, QString top_mid);

	// super patch in unit
	bool isSuperPatchInUnit(PatchNode* superPatch, UnitScaff* unit);
};


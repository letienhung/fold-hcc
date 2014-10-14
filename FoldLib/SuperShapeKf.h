#pragma once

#include "Scaffold.h"

class UnitScaff;

// SuperShapeKf is the snapshot of the entire object at certain time
// super patches are used to replace the clasped parts

class SuperShapeKf final : public Scaffold
{
public:
	SuperShapeKf(Scaffold* superKeyframe, StringSetMap moc_g);

	// valid only if all order constraints are met
	bool isValid(Vector3 sqzV);

	// obstacles
	void computeObstacles(UnitScaff* unit, QString base_mid, QString top_mid,
						QVector<Vector3>& obstPnts, QVector<Vector2>& obstPntsProj);

private:
	// in between parts
	QVector<ScaffNode*> getInbetweenExternalParts(UnitScaff* unit, QString base_mid, QString top_mid);

	// unrelated masters
	QVector<ScaffNode*> getUnrelatedExternalMasters(UnitScaff* unit, QString base_mid, QString top_mid);

	// super patch in unit
	bool unitContainsNode(UnitScaff* unit, Structure::Node* node);

public:
	// the map between master and super master
	QMap<QString, QString> master2SuperMap;

	// the oder constrains of super masters
	StringSetMap mocGreater, mocLess;
};


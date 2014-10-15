#pragma once

#include "Scaffold.h"

class UnitScaff;

// SuperShapeKf is the snapshot of the entire object at certain time
// super patches are used to replace the clasped parts

class SuperShapeKf final : public Scaffold
{
public:
	SuperShapeKf(Scaffold* superKeyframe, StringSetMap moc_g, Vector3 sqzV);

	// valid only if all order constraints are met
	bool isValid();

	// parts in between two regular masters
	QVector<ScaffNode*> getInbetweenParts(QString base_mid, QString top_mid);

	// unrelated masters with the super master of the regular master
	QVector<ScaffNode*> getUnrelatedMasters(QString regular_mid);

public:
	// the map between master and super master
	QMap<QString, QString> master2SuperMap;

	// the oder constrains using super masters
	StringSetMap mocGreater, mocLess;

	// the squeezing direction
	Vector3 sqzV;
};


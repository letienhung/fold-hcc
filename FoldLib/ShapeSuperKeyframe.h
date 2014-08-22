#pragma once

#include "FdGraph.h"

// This class represents a key frame of the shape under certain time
// Different from regular key frame, this class introduce "super master"
// to represent collapsed/merged masters and their mapping relations as well
// The master order constrains are also updated for super masters
// These additional information helps compute various folding volumes

class ShapeSuperKeyframe : public FdGraph
{
public:
	ShapeSuperKeyframe(FdGraph* superKeyframe, StringSetMap moc_g);
	// the map between master and super master
	QMap<QString, QString> master2SuperMap;

	// update the master oder constrains with super masters
	StringSetMap mocGreater, mocLess;

	// A FdGraph is valid only if all order constraints are met
	bool isValid(Vector3 sqzV);
};


#pragma once

#include "FdUtility.h"

class ShapeScaffold;

class Decomposer
{
public:
	Decomposer(ShapeScaffold* ss);
	void execute();

private:
	// decomposition
	FdNodeArray2D getPerpConnGroups();
	void createMasters();

	void updateSlaves();
	void updateSlaveMasterRelation();
	void createSlaves();

	void clusterSlaves();

private:
	ShapeScaffold* scfd;
};


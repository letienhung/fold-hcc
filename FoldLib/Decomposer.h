#pragma once

#include "FdUtility.h"

class DecScaff;

class Decomposer
{
public:
	Decomposer(DecScaff* ss);
	void execute();

private:
	// decomposition
	ScaffNodeArray2D getPerpConnGroups();
	void createMasters();

	void updateSlaves();
	void updateSlaveMasterRelation();
	void createSlaves();

	void clusterSlaves();

private:
	DecScaff* scfd;
};


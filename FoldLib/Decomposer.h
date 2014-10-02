#pragma once

#include "FdUtility.h"

class DecScaffold;

class Decomposer
{
public:
	Decomposer(DecScaffold* ss);
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
	DecScaffold* scfd;
};


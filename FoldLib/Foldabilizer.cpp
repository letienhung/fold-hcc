#include "Foldabilizer.h"

Foldabilizer::Foldabilizer()
{
	scaffold = NULL;
	selectedId = 0;
	pushAxis = 2;
}

void Foldabilizer::setScaffold( FdGraph* fdg )
{
	scaffold = fdg;

	// reset
	foreach(LayerModel* lm, layerModels)
		delete lm;
	layerModels.clear();
}

void Foldabilizer::fold()
{
	if (pushAxis < 3)
	{
		Vector3 direct(0, 0, 0);
		direct[pushAxis] = 1;;
		layerModels.push_back(new LayerModel(scaffold, direct));
	}
	else
	{
		layerModels.push_back(new LayerModel(scaffold, Vector3(1, 0, 0)));
		layerModels.push_back(new LayerModel(scaffold, Vector3(0, 1, 0)));
		layerModels.push_back(new LayerModel(scaffold, Vector3(0, 0, 1)));
	}
}


void Foldabilizer::setPushAxis( int d )
{
	pushAxis = d;
}

FdGraph* Foldabilizer::selectedScaffold()
{
	if (selectedId >= 0 && selectedId < layerModels.size())
		return layerModels[selectedId]->layerGraph;
	else
		return NULL;
}


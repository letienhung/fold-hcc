#pragma once

#include "ChainGraph.h"

class LayerGraph : public FdGraph
{
public:
	enum LAYER_TYPE{PIZZA, SANDWICH};

public:
    LayerGraph(QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox);
	~LayerGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	ChainGraph* getChain(QString cid);

	// foldabilize
	virtual void foldabilize() = 0;

	// fold option graph
	void exportFOG();

	// key frames
	virtual void snapshot(double t);
	virtual QVector<Structure::Node*> getKeyFrameNodes(double t) = 0;

public:
	LAYER_TYPE mType;
	int selId;
	QVector<ChainGraph*> chains;

	FoldOptionGraph* fog;
	Geom::Box barrierBox;
	QVector<FoldOptionGraph*> fogs;
}; 


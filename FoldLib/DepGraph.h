#pragma once
#include "Graph.h"
#include "PizzaChain.h"

enum FD_DIRECTION{FD_LEFT, FD_RIGHT};

class PizzaDyGraph : public Structure::Graph
{
public:
    PizzaDyGraph();
	PizzaDyGraph(PizzaDyGraph& other);
	Graph* clone();

public:

};



class PizzaPNode : public Structure::Node
{
public:

	// part info
	PizzaChain* chain;

};

class PizzaFNode : public Structure::Node
{
public:
	PizzaFNode(int hid, FD_DIRECTION d, QString id);

	// folding info
	int hingeId;
	FD_DIRECTION direct;
	double score;
};


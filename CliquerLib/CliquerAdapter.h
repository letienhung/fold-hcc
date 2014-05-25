#pragma once

#include <QVector>
#include <QMultiMap>
#include "cliquer_graph.h"

class CliquerAdapter
{
public:
    CliquerAdapter(const QVector< QVector<bool> > &m, const QVector<double>& w);
	~CliquerAdapter();

	QVector<QVector<int> > getMinWeightMaxCliques();
	QVector<QVector<int> > getMaxWeightedCliques();
	
private:
	graph_t *graph;
	int *weights, *weightsAllOne;
};


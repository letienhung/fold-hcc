#pragma once

#include <QVector>
#include <QMultiMap>
#include "cliquer_graph.h"

class CliquerAdapter
{
public:
    CliquerAdapter(const QVector< QVector<bool> > &m, const QVector<double>& w);
	~CliquerAdapter();

	double weightOf(const QVector<int> &clique);
	void computeWeightsOfAllMaxCliques();
	QVector<QVector<int> > getMinWeightMaxCliques();
	
private:
	graph_t* g;
	QVector<double> weights;
	QMultiMap< double, QVector<int> > weight_clique_map;
};


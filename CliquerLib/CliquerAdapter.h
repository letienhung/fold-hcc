#pragma once

#include <QVector>
#include "cliquer_graph.h"

class CliquerAdapter
{
public:
    CliquerAdapter(QVector< QVector<bool> > &m);
	~CliquerAdapter();

	QVector< QVector<int> > getAllMaximumCliques();

private:
	graph_t* g;
};


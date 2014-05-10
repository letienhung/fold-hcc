#include "CliquerAdapter.h"
#include "cliquer.h"
#include <iostream>


CliquerAdapter::CliquerAdapter( const QVector< QVector<bool> > &m, const QVector<double>& w )
{
	// create graph
	graph = (graph_t*)calloc(1,sizeof(graph_t));
	graph->n = m.size();

	// edges
	graph->edges = (set_t*)calloc(graph->n,sizeof(set_t));
	for (int i = 0; i < graph->n; i++)
		graph->edges[i] = set_new(graph->n);

	for (int i = 0; i < graph->n; i++){
		for (int j = i+1; j < graph->n; j++)
			if (m[i][j]) GRAPH_ADD_EDGE(graph, i, j);
	}

	// weights: int to double (multiplying 100)
	// weightsAllOne: fake weights for unweighted graph
	weights = (int*)calloc(graph->n,sizeof(int));
	weightsAllOne = (int*)calloc(graph->n,sizeof(int));
	for (int i = 0; i < graph->n; i++)
	{
		weights[i] = int(w[i] * 100);
		weightsAllOne[i] = 1;
	}
}

CliquerAdapter::~CliquerAdapter()
{
	// delete the graph
	if (graph)	graph_free(graph);
}


QVector<QVector<int> > CliquerAdapter::getMinWeightMaxCliques()
{
	graph_t *unweighted_graph = graph;
	unweighted_graph->weights = weightsAllOne;

	// the size of max clique
	int max_size = clique_max_weight(unweighted_graph, NULL);
	
	// weighted graph
	graph_t *weighted_graph = graph;
	weighted_graph->weights = weights;

	// mwmc list
	set_t *mwmc_list;

	// find min weight max clique
	int mwmc_list_count = clique_mwmc_find_all(weighted_graph, max_size, &mwmc_list);

	// extract cliques
	std::cout << "\nreading cliques\n";
	QVector<QVector<int> > mwmc;
	for (int i = 0; i < mwmc_list_count; i++)
	{
		// clique in byte format
		set_t clique = mwmc_list[i];
		set_print(clique);

		// read clique
		QVector<int> qc;
		for (int j = 0; j < graph->n; j++)
			if (SET_CONTAINS(clique, j)) qc << j;

		// store clique
		mwmc << qc;
	}

	return mwmc;
}

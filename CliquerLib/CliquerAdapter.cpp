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
	int max_clique_size = clique_max_weight(unweighted_graph, NULL);
	
	// weighted graph
	graph_t *weighted_graph = graph;
	weighted_graph->weights = weights;

	// mwmc list
	int mwmc_list_length = 1024;
	int **mwmc_list = new int*[mwmc_list_length];
	for (int i = 0; i < mwmc_list_length; i++)
		mwmc_list[i] = new int[max_clique_size];

	// find min weight max clique
	int mwmc_list_count = clique_mwmc_find_all(weighted_graph, max_clique_size, 
												&mwmc_list, mwmc_list_length);

	// extract cliques
	std::cout << "\nreading cliques\n";
	QVector<QVector<int> > mwmc;
	for (int i = 0; i < mwmc_list_count; i++)
	{
		QVector<int> clique;
		for (int j = 0; j < max_clique_size; j++)
		{
			std::cout << mwmc_list[i][j] << "  ";
			clique << mwmc_list[i][j];
		}
		std::cout << std::endl;

		mwmc << clique;
	}

	// delete 
	for (int i = 0; i < mwmc_list_length; i++)
		delete [] mwmc_list[i];
	delete [] mwmc_list;

	return mwmc;
}

QVector<QVector<int> > CliquerAdapter::getMaxWeightedCliques()
{
	// weighted graph
	graph->weights = weights;

	// call Cliquer
	set_t s[1024];
	clique_default_options->clique_list=s;
	clique_default_options->clique_list_length=1024;
	int n = clique_find_all(graph,0,0,true,NULL);

	// read
	std::cout << "#MWC = " << n << "\n";
	QVector<QVector<int> > qCliques;
	for (int ci = 0; ci < n; ci++)
	{
		set_t clique = s[ci];

		QVector<int> qClique;
		int w = 0;
		for (int i = 0; i < graph->n; i++)
		{
			if (SET_CONTAINS(clique,i))
			{
				qClique << i;
				w += weights[i];
				std::cout << i << " ";
			}
		}

		std::cout << "\t: w = " << w << "\n";

		qCliques << qClique;
		//set_free(clique);
	}

	return qCliques;
}

#include "CliquerAdapter.h"
#include "cliquer.h"

CliquerAdapter::CliquerAdapter( QVector< QVector<bool> > &m )
{
	// create graph
	g = (graph_t*)calloc(1,sizeof(graph_t));
	g->n = m.size();

	// edges
	g->edges = (set_t*)calloc(g->n,sizeof(set_t));
	for (int i = 0; i < g->n; i++)
		g->edges[i] = set_new(g->n);

	for (int i = 0; i < g->n; i++){
		for (int j = i+1; j < g->n; j++)
			if (m[i][j]) GRAPH_ADD_EDGE(g, i, j);
	}

	// weights
	// nodes in unweighted graph have the same weight
	g->weights = (int*)calloc(g->n,sizeof(int));
	for (int i = 0; i < g->n; i++)
		g->weights[i] = 1;
}

CliquerAdapter::~CliquerAdapter()
{
	// delete the graph
	if (g)	graph_free(g);
}

QVector< QVector<int> > CliquerAdapter::getAllMaximumCliques()
{
	// clique list
	set_t s[1024];
	clique_default_options->clique_list = s;
	clique_default_options->clique_list_length = 1024;

	// find all max cliques
	int max_size = clique_max_weight(g, NULL);
	int n = clique_unweighted_find_all(g, max_size, max_size, false, NULL);
	clique_default_options->clique_list = NULL;

	// output
	QVector< QVector<int> > maxCliques(n);
	for (int i = 0; i < n; i++){
		for (int j = 0; j < g->n; j++)
		{
			if (SET_CONTAINS(s[i], j))
				maxCliques[i].push_back(j);
		}
	}
	return maxCliques;
}

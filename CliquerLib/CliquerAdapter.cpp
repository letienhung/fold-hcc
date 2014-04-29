#include "CliquerAdapter.h"
#include "cliquer.h"


CliquerAdapter::CliquerAdapter( const QVector< QVector<bool> > &m, const QVector<double>& w )
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

	// fake weights for graph
	// nodes in unweighted graph have the same weight
	g->weights = (int*)calloc(g->n,sizeof(int));
	for (int i = 0; i < g->n; i++)
		g->weights[i] = 1;

	// real weights
	weights = w;
}

CliquerAdapter::~CliquerAdapter()
{
	// delete the graph
	if (g)	graph_free(g);
}


double CliquerAdapter::weightOf( const QVector<int> &clique )
{
	double w = 0;
	foreach(int id, clique) w += weights[id];
	return w;
}


void CliquerAdapter::computeWeightsOfAllMaxCliques()
{
	// clique list
	set_t s[4096];
	clique_default_options->clique_list = s;
	clique_default_options->clique_list_length = 4096;

	// find all max cliques
	int max_size = clique_max_weight(g, NULL);
	int n = clique_unweighted_find_all(g, max_size, max_size, false, NULL);
	clique_default_options->clique_list = NULL;

	// compute weight of each clique and store them
	// in case there are too many max cliques, only the first 1024 are considerded.
	int nn = (n > 4096)? 4096 : n;
	for (int i = 0; i < nn; i++)
	{
		// get max clique
		QVector<int> maxClique;
		for (int j = 0; j < g->n; j++)
		{
			if (SET_CONTAINS(s[i], j))
				maxClique.push_back(j);
		}

		// compute weight
		double w = weightOf(maxClique);
		weight_clique_map.insert(w, maxClique);
	}
}

QVector<int> CliquerAdapter::getMinWeightMaxClique()
{
	return *(weight_clique_map.begin());
}


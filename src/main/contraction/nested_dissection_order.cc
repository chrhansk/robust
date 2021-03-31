#include "nested_dissection_order.hh"

#include <cassert>
#include <cstdlib>

#include <iostream>
#include <stdexcept>

#include "graph/metis_graph.hh"
#include "graph/vertex_set.hh"

NestedDissectionOrder::NestedDissectionOrder(const Graph& graph)
  : VertexMap(graph, 0)
{
  METIS::idx_t* perm;
  METIS::idx_t* iperm;

  METISGraph simpleGraph(graph);

  METIS::idx_t n = simpleGraph.getNumVertices();

  perm = (METIS::idx_t*) malloc(sizeof(METIS::idx_t) * n);
  iperm = (METIS::idx_t*) malloc(sizeof(METIS::idx_t) * n);

  METIS::idx_t options[METIS_NOPTIONS];
  METIS::METIS_SetDefaultOptions(options);

  options[METIS::METIS_OPTION_SEED] = 17;
  options[METIS::METIS_OPTION_DBGLVL] = METIS::METIS_DBG_INFO;
  options[METIS::METIS_OPTION_NUMBERING] = 0;

  int value = METIS::METIS_NodeND(&n,
                                  simpleGraph.getXAdj(),
                                  simpleGraph.getAdjncy(),
                                  NULL, /* vertex weights */
                                  options,
                                  perm,
                                  iperm);

  if(value != METIS::METIS_OK)
  {
    throw std::runtime_error("Could not start METIS");
  }

  for(const Vertex& vertex : graph.getVertices())
  {
    (*this)(vertex) = iperm[vertex.getIndex()];
  }

  assert(check(graph));

  free(iperm);
  free(perm);
}

bool NestedDissectionOrder::check(const Graph& graph) const
{
  VertexSet found(graph);

  for(const Vertex& vertex : graph.getVertices())
  {
    idx value = (*this)(vertex);

    if(value >= graph.getVertices().size())
    {
      return false;
    }

    const Vertex& other = graph.getVertices()[value];

    if(found.contains(other))
    {
      return false;
    }

    found.insert(other);
  }

  for(const Vertex& vertex : graph.getVertices())
  {
    if(!found.contains(vertex))
    {
      return false;
    }
  }

  return true;
}

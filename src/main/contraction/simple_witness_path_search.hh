#ifndef SIMPLE_WITNESS_PATH_SEARCH_HH
#define SIMPLE_WITNESS_PATH_SEARCH_HH

#include "contraction_preprocessor.hh"

#include "witness_path_search.hh"

/**
 * The SimpleWitnessPath computes the ContracionPair%s of a Vertex
 * by calling a BidirectionalRouter for each pair of
 * adjacent vertices.
 **/
class SimpleWitnessPathSearch : public WitnessPathSearch
{
public:
  SimpleWitnessPathSearch(const Graph& graph,
                          const EdgeFunc<num>& costs,
                          const VertexFunc<bool>& contracted)
    : WitnessPathSearch(graph, costs, contracted)
  {}

  std::vector<ContractionPair> findPairs(Vertex vertex) const override;
};


#endif /* SIMPLE_WITNESS_PATH_SEARCH_HH */

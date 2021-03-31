#ifndef FAST_WITNESS_PATH_SEARCH_HH
#define FAST_WITNESS_PATH_SEARCH_HH

#include "contraction_preprocessor.hh"

#include "witness_path_search.hh"

/**
 * The FastWitnessPathSearch computes the ContractionPair%s of
 * a Vertex by computing partial shortest path trees for
 * adjacent vertices. This is usually faster than the
 * SimpleWitnessPathSearch.
 **/
class FastWitnessPathSearch : public WitnessPathSearch
{
private:
  template <Direction direction, class OutIt>
  void findPairs(const Vertex vertex,
                 const Edge edge,
                 const std::vector<Edge>& edges,
                 OutIt it) const;

public:
  FastWitnessPathSearch(const Graph& graph,
                        const EdgeFunc<num>& costs,
                        const VertexFunc<bool>& contracted)
    : WitnessPathSearch(graph, costs, contracted)
  {}

  std::vector<ContractionPair> findPairs(Vertex vertex) const override;
};

#endif /* FAST_WITNESS_PATH_SEARCH_HH */

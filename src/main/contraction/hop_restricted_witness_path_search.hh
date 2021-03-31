#ifndef HOP_RESTRICTED_WITNESS_PATH_SEARCH_HH
#define HOP_RESTRICTED_WITNESS_PATH_SEARCH_HH

#include "contraction_preprocessor.hh"

#include "witness_path_search.hh"

/**
 * The FastWitnessPathSearch computes the ContractionPair%s of
 * a Vertex by computing partial shortest path trees for
 * adjacent vertices. Additionaly the search is restricted
 * to a limited number of hops. i.e. to paths containing
 * a limited number of edges.
 **/
class HopRestrictedWitnessPathSearch : public WitnessPathSearch
{
private:
  const idx hopLimit;

  template <Direction direction, class OutIt>
  void findPairs(const Vertex vertex,
                 const Edge edge,
                 const std::vector<Edge>& edges,
                 OutIt it) const;

public:
  HopRestrictedWitnessPathSearch(const Graph& graph,
                                 const EdgeFunc<num>& costs,
                                 const VertexFunc<bool>& contracted,
                                 idx hopLimit);

  std::vector<ContractionPair> findPairs(Vertex vertex) const override;
};

#endif /* HOP_RESTRICTED_WITNESS_PATH_SEARCH_HH */

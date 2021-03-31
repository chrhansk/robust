#ifndef FAST_ROBUST_WITNESS_PATH_SEARCH_HH
#define FAST_ROBUST_WITNESS_PATH_SEARCH_HH

#include "robust_witness_path_search.hh"
#include "robust_contraction_preprocessor.hh"

class FastRobustWitnessPathSearch : public RobustWitnessPathSearch
{
private:
  template <Direction direction, class OutIt>
  void findPairs(const Vertex vertex,
                 const Edge edge,
                 const std::vector<Edge>& edges,
                 OutIt outIt) const;

public:
  FastRobustWitnessPathSearch(const Graph& graph,
                              const EdgeFunc<const ContractionRange&>& contractionRanges,
                              const ValueVector& values,
                              const VertexFunc<bool>& contracted)
    : RobustWitnessPathSearch(graph, contractionRanges, values, contracted)
  {}

  virtual std::vector<RobustContractionPair> findPairs(Vertex vertex) const override;
};


#endif /* FAST_ROBUST_WITNESS_PATH_SEARCH_HH */

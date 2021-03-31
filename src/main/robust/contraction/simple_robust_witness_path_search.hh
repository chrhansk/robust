#ifndef SIMPLE_ROBUST_WITNESS_PATH_SEARCH_HH
#define SIMPLE_ROBUST_WITNESS_PATH_SEARCH_HH

#include "robust_witness_path_search.hh"
#include "robust_contraction_preprocessor.hh"

class SimpleRobustWitnessPathSearch : public RobustWitnessPathSearch
{
private:

  template <class OutIt>
  void findPair(const Vertex vertex,
                const Edge& incoming,
                const Edge& outgoing,
                OutIt outIt) const;

public:
  SimpleRobustWitnessPathSearch(const Graph& graph,
                                const EdgeFunc<const ContractionRange&>& contractionRanges,
                                const ValueVector& values,
                                const VertexFunc<bool>& contracted)
    : RobustWitnessPathSearch(graph, contractionRanges, values, contracted)
  {}

  virtual std::vector<RobustContractionPair> findPairs(Vertex vertex) const override;
};


#endif /* SIMPLE_ROBUST_WITNESS_PATH_SEARCH_HH */

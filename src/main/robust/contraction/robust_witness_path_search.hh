#ifndef ROBUST_WITNESS_PATH_SEARCH_HH
#define ROBUST_WITNESS_PATH_SEARCH_HH

#include <vector>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "contraction_range.hh"
#include "robust_contraction_pair.hh"

class RobustWitnessPathSearch
{
protected:
  const Graph& graph;
  const EdgeFunc<const ContractionRange&>& contractionRanges;
  const ValueVector& values;
  const VertexFunc<bool>& contracted;

public:
  RobustWitnessPathSearch(const Graph& graph,
                          const EdgeFunc<const ContractionRange&>& contractionRanges,
                          const ValueVector& values,
                          const VertexFunc<bool>& contracted)
    : graph(graph),
      contractionRanges(contractionRanges),
      values(values),
      contracted(contracted)
  {}

  virtual std::vector<RobustContractionPair> findPairs(Vertex vertex) const = 0;
};


#endif /* ROBUST_WITNESS_PATH_SEARCH_HH */

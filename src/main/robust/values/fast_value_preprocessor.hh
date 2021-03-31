#ifndef FAST_VALUE_PREPROCESSOR_HH
#define FAST_VALUE_PREPROCESSOR_HH

#include <unordered_set>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"

#include "router/label.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"

#include "abstract_value_preprocessor.hh"

class FastValuePreprocessor : public AbstractValuePreprocessor
{
private:
  void updateBounds(const Region& sourceRegion,
                    const Region& targetRegion,
                    VertexPairMap<BoundValues>& globalBounds,
                    const VertexPairMap<Bound>& currentBounds,
                    num value) const;

  void setLowerBounds(const Region& sourceRegion,
                      const Region& targetRegion,
                      const std::unordered_set<Vertex>& sourceBoundaries,
                      const std::unordered_set<Vertex>& targetBoundaries,
                      const DistanceMap& boundaryDistances,
                      VertexPairMap<Bound>& currentBounds,
                      idx i) const;


public:
  FastValuePreprocessor(const Graph& graph,
                        const EdgeFunc<num>& costs,
                        const EdgeFunc<num>& deviations,
                        num deviationSize,
                        const Partition& partition)
    : AbstractValuePreprocessor(graph, costs, deviations, deviationSize, partition)
  {}


  std::unordered_set<num> requiredValues(const Region& sourceRegion,
                                         const Region& targetRegion) const;
};


#endif /* FAST_VALUE_PREPROCESSOR_HH */

#ifndef DISCARDING_PREPROCESSOR_HH
#define DISCARDING_PREPROCESSOR_HH

#include <unordered_map>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"
#include "arcflags/region_pair_map.hh"

#include "robust/robust_utils.hh"

#include "robust/arcflags/simple_arcflags.hh"

class DiscardingPreprocessor
{
private:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const Partition& partition;

  std::unordered_map<num, std::vector<Edge>> valueEdges;
  ValueSet bestValues;
  RegionPairMap<num> distances;
  Bidirected<SimpleArcFlags> arcFlags;

public:
  DiscardingPreprocessor(const Graph& graph,
                         const EdgeFunc<num>& costs,
                         const EdgeFunc<num>& deviations,
                         const Partition& partition);

  bool canDiscard(Vertex source,
                  Vertex target,
                  num value,
                  idx deviationSize,
                  num upperBound) const;

  const Bidirected<SimpleArcFlags>& getArcFlags() const
  {
    return arcFlags;
  }

  const ValueSet& getBestValues() const
  {
    return bestValues;
  }

  const Partition& getPartition() const
  {
    return partition;
  }
};

#endif /* DISCARDING_PREPROCESSOR_HH */

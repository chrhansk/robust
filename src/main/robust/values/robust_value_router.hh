#ifndef ROBUST_VALUE_ROUTER_HH
#define ROBUST_VALUE_ROUTER_HH

#include "robust/robust_router.hh"

#include "arcflags/partition.hh"
#include "arcflags/region_pair_map.hh"

#include "robust/robust_utils.hh"

class RobustValueRouter : public RobustRouter
{
private:
  const Graph& graph;
  RobustRouter& router;
  const Partition& partition;
  const RegionPairMap<ValueVector>& requiredValues;

public:
  RobustValueRouter(const EdgeFunc<num>& costs,
                    const EdgeFunc<num>& deviations,
                    idx deviationSize,
                    RobustRouter& router,
                    const Partition& partition,
                    const RegionPairMap<ValueVector>& requiredValues)
    : RobustRouter(partition.getGraph(), costs, deviations, deviationSize),
      graph(partition.getGraph()),
      router(router),
      partition(partition),
      requiredValues(requiredValues)
  {}

  RobustSearchResult shortestPath(Vertex source,
                                  Vertex target,
                                  const ValueVector& possibleValues,
                                  num bound) override;
};


#endif /* ROBUST_VALUE_ROUTER_HH */

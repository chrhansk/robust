#include "robust_value_router.hh"

RobustSearchResult
RobustValueRouter::shortestPath(Vertex source,
                                Vertex target,
                                const ValueVector& possibleValues,
                                num bound)
{
  const Region& sourceRegion = partition.getRegion(source);
  const Region& targetRegion = partition.getRegion(target);

  if(sourceRegion == targetRegion)
  {
    return router.shortestPath(source, target, values, bound);
  }
  else
  {
    return router.shortestPath(source, target, requiredValues(sourceRegion, targetRegion), bound);
  }
}

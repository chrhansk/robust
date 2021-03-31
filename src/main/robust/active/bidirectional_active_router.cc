#include "bidirectional_active_router.hh"

#include "robust/reduced_costs.hh"

ActiveRouter::ActiveSearchResult
BidirectionalActiveRouter::findShortestPath(Vertex source,
                                            Vertex target,
                                            num value)
{
  return findShortestPath(source, target, value,
                          AllEdgeFilter(),
                          AllEdgeFilter());
}

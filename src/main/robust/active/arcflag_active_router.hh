#ifndef ARCFLAG_ACTIVE_ROUTER_H
#define ARCFLAG_ACTIVE_ROUTER_H

#include "active_router.hh"
#include "bidirectional_active_router.hh"

#include "robust/arcflags/simple_arcflags.hh"

#include "router/router.hh"

class ArcFlagActiveRouter : public ActiveRouter
{
private:
  const Partition& partition;
  const SimpleArcFlags& incomingFlags;
  const SimpleArcFlags& outgoingFlags;
  BidirectionalActiveRouter router;

public:
  ArcFlagActiveRouter(const Graph& graph,
                      const EdgeFunc<num>& costs,
                      const EdgeFunc<num>& deviations,
                      idx deviationSize,
                      const Partition& partition,
                      const Bidirected<SimpleArcFlags>& flags)
    : ActiveRouter(graph, costs, deviations, deviationSize),
      partition(partition),
      incomingFlags(flags.get(Direction::INCOMING)),
      outgoingFlags(flags.get(Direction::OUTGOING)),
      router(graph, costs, deviations, deviationSize)
  {}

  ArcFlagActiveRouter(const Graph& graph,
                      const EdgeFunc<num>& costs,
                      const EdgeFunc<num>& deviations,
                      idx deviationSize,
                      const Partition& partition,
                      const SimpleArcFlags& incomingFlags,
                      const SimpleArcFlags& outgoingFlags)
    : ActiveRouter(graph, costs, deviations, deviationSize),
      partition(partition),
      incomingFlags(incomingFlags),
      outgoingFlags(outgoingFlags),
      router(graph, costs, deviations, deviationSize)
  {}

  ActiveSearchResult findShortestPath(Vertex source, Vertex target, num theta) override;
};

ActiveRouter::ActiveSearchResult
ArcFlagActiveRouter::findShortestPath(Vertex source, Vertex target, num theta)
{
  const Region& sourceRegion = partition.getRegion(source);
  const Region& targetRegion = partition.getRegion(target);

  if(sourceRegion == targetRegion)
  {
    return router.findShortestPath(source,
                                   target,
                                   theta,
                                   AllEdgeFilter(),
                                   AllEdgeFilter());
  }

  SimpleArcFlags::ThetaFilter forwardFilter =
    incomingFlags.getThetaFilter(targetRegion, theta);

  SimpleArcFlags::ThetaFilter backwardFilter =
    outgoingFlags.getThetaFilter(sourceRegion, theta);

  return router.findShortestPath(source,
                                 target,
                                 theta,
                                 forwardFilter,
                                 backwardFilter);
}

#endif /* ARCFLAG_ACTIVE_ROUTER_H */

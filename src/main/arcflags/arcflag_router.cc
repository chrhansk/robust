#include "arcflag_router.hh"


ArcFlagRouter::ArcFlagRouter(const Graph& graph,
                             const ArcFlags& forwardFlags,
                             const ArcFlags& backwardFlags,
                             const Partition& partition)
  : BidirectionalRouter(graph),
    forwardFlags(forwardFlags),
    backwardFlags(backwardFlags),
    partition(partition)
{
}

SearchResult ArcFlagRouter::shortestPath(Vertex source,
                                         Vertex target,
                                         const EdgeFunc<num>& costs)
{
  ArcFlags::Filter forwardFilter(backwardFlags.getFilter(target));
  ArcFlags::Filter backwardFilter(forwardFlags.getFilter(source));

  return BidirectionalRouter::shortestPath<ArcFlags::Filter,
                                           ArcFlags::Filter,
                                           false>(source,
                                                  target,
                                                  costs,
                                                  forwardFilter,
                                                  backwardFilter);
}

SearchResult ArcFlagRouter::shortestPath(Vertex source,
                                         Vertex target,
                                         const EdgeFunc<num>& costs,
                                         num bound)
{
  ArcFlags::Filter forwardFilter(backwardFlags.getFilter(target));
  ArcFlags::Filter backwardFilter(forwardFlags.getFilter(source));

  return BidirectionalRouter::shortestPath<ArcFlags::Filter,
                                           ArcFlags::Filter,
                                           true>(source,
                                                 target,
                                                 costs,
                                                 forwardFilter,
                                                 backwardFilter,
                                                 bound);
}

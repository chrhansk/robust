#ifndef ABSTRACT_ARCFLAG_ROUTER_HH
#define ABSTRACT_ARCFLAG_ROUTER_HH

#include "arcflags.hh"
#include "partition.hh"

#include "router/router.hh"
#include "router/bidirectional_router.hh"

/**
 * A router which computes shortest paths using
 * a bidirectional search according
 * to ArcFlags determined by an ArcFlagPreprocessor.
 **/
class ArcFlagRouter : public Router,
                      protected BidirectionalRouter
{
protected:
  const ArcFlags& forwardFlags;
  const ArcFlags& backwardFlags;
  const Partition& partition;

public:
  ArcFlagRouter(const Graph& graph,
                const ArcFlags& forwardFlags,
                const ArcFlags& backwardFlags,
                const Partition& partition);

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs) override;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs,
                            num bound) override;
};

#endif /* ABSTRACT_ARCFLAG_ROUTER_HH */

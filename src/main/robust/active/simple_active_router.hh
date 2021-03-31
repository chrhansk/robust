#ifndef SIMPLE_ACTIVE_EDGE_ROUTER_HH
#define SIMPLE_ACTIVE_EDGE_ROUTER_HH

#include "active_router.hh"

class SimpleActiveRouter : public ActiveRouter
{
public:
  SimpleActiveRouter(const Graph& graph,
                     const EdgeFunc<num>& costs,
                     const EdgeFunc<num>& deviations,
                     idx deviationSize)
    : ActiveRouter(graph, costs, deviations, deviationSize)
  {}

  ActiveSearchResult findShortestPath(Vertex source, Vertex target, num value) override;
};


#endif /* SIMPLE_ACTIVE_EDGE_ROUTER_HH */

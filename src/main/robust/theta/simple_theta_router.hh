#ifndef SIMPLE_THETA_ROUTER_HH
#define SIMPLE_THETA_ROUTER_HH

#include "theta_router.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "router/bidirectional_router.hh"

class SimpleThetaRouter : public ThetaRouter,
                          protected BidirectionalRouter
{
private:
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;

public:
  SimpleThetaRouter(const Graph& graph,
                    const EdgeFunc<num>& costs,
                    const EdgeFunc<num>& deviations,
                    idx deviationSize);

  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta,
                                    num bound);

  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta);

};



#endif /* SIMPLE_THETA_ROUTER_HH */

#ifndef BIDIRECTIONAL_BOUNDING_ROUTER_HH
#define BIDIRECTIONAL_BOUNDING_ROUTER_HH

#include "stateful_theta_router.hh"

class BidirectionalBoundingRouter : public StatefulThetaRouter
{
private:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const idx deviationSize;
  VertexMap<num> forwardBounds, backwardBounds;

  template <bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num theta,
                                num bound);

public:
  BidirectionalBoundingRouter(const Graph& graph,
                              const EdgeFunc<num>& costs,
                              const EdgeFunc<num>& deviations,
                              idx deviationSize);

  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta,
                                    num bound) override;

  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta) override;

  void doReset() override;
};


#endif /* BIDIRECTIONAL_BOUNDING_ROUTER_HH */

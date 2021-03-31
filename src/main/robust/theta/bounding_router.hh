#ifndef BOUNDING_ROUTER_HH
#define BOUNDING_ROUTER_HH

#include "stateful_theta_router.hh"

/**
 * The bounding router maintains individual
 * upper bounds on the objective function
 * vlaue for each vertex in order to
 * prune the search.
 *
 * Attention: This router is *stateful*
 * and therefore not reentrant.
 **/
class BoundingRouter : public StatefulThetaRouter
{
private:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const idx deviationSize;
  VertexMap<num> upperBounds;

  template <bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num theta,
                                num bound);

public:
  BoundingRouter(const Graph& graph,
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


#endif /* BOUNDING_ROUTER_HH */

#ifndef GOAL_DIRECTED_BOUNDING_ROUTER_HH
#define GOAL_DIRECTED_BOUNDING_ROUTER_HH

#include "goal_directed_router.hh"

class GoalDirectedBoundingRouter : public GoalDirectedRouter
{
private:
  idx deviationSize;
  VertexMap<num> upperBounds;

  template<bool bounded>
  SearchResult computeShortestPath(Vertex source,
                                   Vertex target,
                                   num theta,
                                   num bound);

  template<bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num theta,
                                num bound);

public:
  GoalDirectedBoundingRouter(const Graph& graph,
                             const EdgeFunc<num>& costs,
                             const EdgeFunc<num>& deviations,
                             idx deviationSize);

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta,
                            num bound) override;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta) override;

  void doReset() override;
};


#endif /* GOAL_DIRECTED_BOUNDING_ROUTER_HH */

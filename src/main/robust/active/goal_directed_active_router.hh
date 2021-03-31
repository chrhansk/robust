#ifndef ALTERNATING_ACTIVE_ROUTER_HH
#define ALTERNATING_ACTIVE_ROUTER_HH

#include "active_router.hh"

#include "robust/theta/goal_directed_router.hh"

class GoalDirectedActiveRouter : public ActiveRouter,
                                 protected GoalDirectedRouter
{
private:

  using ActiveRouter::graph;
  using ActiveRouter::costs;
  using ActiveRouter::deviations;

public:
  GoalDirectedActiveRouter(const Graph& graph,
                           const EdgeFunc<num>& costs,
                           const EdgeFunc<num>& deviations,
                           idx deviationSize)
    : ActiveRouter(graph, costs, deviations, deviationSize),
      GoalDirectedRouter(graph, costs, deviations, deviationSize)
  {
  }

  ActiveSearchResult findShortestPath(Vertex source, Vertex target, num value) override;
  ActiveSearchResult findMaxShortestPath(Vertex source, Vertex target, num value) override;

};


#endif /* ALTERNATING_ACTIVE_ROUTER_HH */

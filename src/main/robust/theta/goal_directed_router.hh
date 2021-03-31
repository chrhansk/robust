#ifndef GOAL_DIRECTED_ROUTER_HH
#define GOAL_DIRECTED_ROUTER_HH

#include "stateful_theta_router.hh"
#include "potential.hh"

class GoalDirectedRouter : public StatefulThetaRouter
{
protected:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;

  PartialDistanceMap<Direction::INCOMING> partialDistances;

  bool shouldRecompute;
  idx lastSettled;

  // chosen according to a parameter test
  float recomputationFactor = 0.15;

  template<bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num theta,
                                num bound);

  template<bool bounded>
  SearchResult recomputePotential(Vertex source,
                                  Vertex target,
                                  num theta,
                                  num bound);

  template<bool bounded>
  SearchResult computeShortestPath(Vertex source,
                                   Vertex target,
                                   num theta,
                                   num bound) const;

public:
  GoalDirectedRouter(const Graph& graph,
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

  float getRecomputationFactor() const
  {
    return recomputationFactor;
  }

  void setRecomputationFactor(float factor)
  {
    assert(factor > 0);
    assert(factor <= 1);

    recomputationFactor = factor;
  }

  void doReset() override;
};


#endif /* GOAL_DIRECTED_ROUTER_HH */

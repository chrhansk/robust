#ifndef BIDIRECTIONAL_GOAL_DIRECTED_ROUTER_HH
#define BIDIRECTIONAL_GOAL_DIRECTED_ROUTER_HH

#include "stateful_theta_router.hh"
#include "router/bidirectional_router.hh"

#include "potential.hh"

class BidirectionalGoalDirectedRouter : public StatefulThetaRouter,
                                        protected BidirectionalRouter
{
public:
  typedef std::function<num (num, num)> PotentialCombination;

  static const PotentialCombination Average, Max, Min;

private:
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;

  PotentialCombination potentialCombination;

  PartialDistanceMap<Direction::OUTGOING> forwardDistances;
  PartialDistanceMap<Direction::INCOMING> backwardDistances;

  bool shouldRecompute;
  idx lastSettled;
  bool extendedRecomputation;

  // chosen according to a parameter test
  float recomputationFactor = 0.6;

  template<bool bounded>
  SearchResult recomputePotential(Vertex source,
                                  Vertex target,
                                  num theta,
                                  num bound);

  template<bool bounded>
  SearchResult computeShortestPath(Vertex source,
                                   Vertex target,
                                   num theta,
                                   num bound);

  template <bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num theta,
                                num bound);

public:
  BidirectionalGoalDirectedRouter(const Graph& graph,
                                  const EdgeFunc<num>& costs,
                                  const EdgeFunc<num>& deviations,
                                  idx deviationsize,
                                  const PotentialCombination& combination = Average,
                                  bool extendedRecomputation = true);

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta,
                            num bound) override;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            num theta) override;

  void doReset() override;

  void setCombinedPotential(const PotentialCombination& newCombination)
  {
    doReset();
    potentialCombination = newCombination;
  }

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

  bool doesExtendedRecomputation() const
  {
    return extendedRecomputation;
  }

  void setExtendedRecomputation(bool value)
  {
    extendedRecomputation = value;
  }

};


#endif /* BIDIRECTIONAL_GOAL_DIRECTED_ROUTER_HH */

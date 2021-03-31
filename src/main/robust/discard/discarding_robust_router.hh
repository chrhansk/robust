#ifndef DISCARDING_ROBUST_ROUTER_HH
#define DISCARDING_ROBUST_ROUTER_HH

#include "robust/searching_robust_router.hh"

#include "discarding_preprocessor.hh"

class DiscardingRobustRouter : public SearchingRobustRouter
{
private:
  const DiscardingPreprocessor& preprocessor;

public:
  DiscardingRobustRouter(const Graph& graph,
                         const EdgeFunc<num>& costs,
                         const EdgeFunc<num>& deviations,
                         idx deviationSize,
                         ThetaRouter& router,
                         const DiscardingPreprocessor& preprocessor,
                         SearchingRobustRouter::Options options = Options());

  using RobustRouter::shortestPath;

  RobustSearchResult shortestPath(Vertex source,
                                  Vertex target,
                                  const ValueVector& possibleValues,
                                  num bound) override;

};

#endif /* DISCARDING_ROBUST_ROUTER_HH */



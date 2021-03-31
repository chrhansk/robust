#ifndef SIMPLE_ROBUST_ROUTER_HH
#define SIMPLE_ROBUST_ROUTER_HH

#include "robust_router.hh"
#include "robust_utils.hh"
#include "theta/theta_router.hh"

class SimpleRobustRouter : public RobustRouter
{
private:
  num getBound(num bound, num bestCost, num value);

  bool useBounds;
  ThetaRouter& router;

public:
  SimpleRobustRouter(const Graph& graph,
                     const EdgeFunc<num>& costs,
                     const EdgeFunc<num>& deviations,
                     idx deviationSize,
                     ThetaRouter& router,
                     bool useBounds = true);

  using RobustRouter::shortestPath;

  RobustSearchResult shortestPath(Vertex source,
                                  Vertex target,
                                  const ValueVector& possibleValues,
                                  num bound) override;

  bool doesUseBounds() const;
  void setUseBounds(bool useBounds);
};

#endif /* SIMPLE_ROBUST_ROUTER_HH */

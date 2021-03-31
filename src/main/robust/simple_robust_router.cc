#include "simple_robust_router.hh"

#include <algorithm>
#include <functional>
#include <iostream>

#include "reduced_costs.hh"
#include "robust_utils.hh"

SimpleRobustRouter::SimpleRobustRouter(const Graph& graph,
                                       const EdgeFunc<num>& costs,
                                       const EdgeFunc<num>& deviations,
                                       idx deviationSize,
                                       ThetaRouter& router,
                                       bool useBounds)
  : RobustRouter(graph, costs, deviations, deviationSize),
    useBounds(useBounds),
    router(router)
{
}

RobustSearchResult SimpleRobustRouter::shortestPath(Vertex source,
                                                    Vertex target,
                                                    const ValueVector& possibleValues,
                                                    num bound)
{
  Path bestPath;
  num bestCost = inf;
  bool found = false;

  RobustSearchResult robustSearchResult;

  for(num value : possibleValues)
  {
    const num upperBound = getBound(bound, bestCost, value);

    SearchResult result = router.shortestPath(source, target,
                                              value, upperBound);

    robustSearchResult.add(result);

    if(!result.found)
    {
      continue;
    }

    found = true;

    num currentCost = deviationSize * value + result.cost;

    if(currentCost < bestCost)
    {
      bestCost = currentCost;
      bestPath = result.path;
    }
  }

  robustSearchResult.found = found;
  robustSearchResult.path = bestPath;

  return robustSearchResult;
}

bool SimpleRobustRouter::doesUseBounds() const
{
  return useBounds;
}

void SimpleRobustRouter::setUseBounds(bool useBounds)
{
  this->useBounds = useBounds;
}

num SimpleRobustRouter::getBound(num bound, num bestCost, num value)
{
  return useBounds ? std::min(bound, bestCost - ((num) deviationSize) * value) : bound;
}

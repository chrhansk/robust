#include "searching_robust_router.hh"

#include <cassert>
#include <iostream>
#include <queue>

#include "router/bidirectional_router.hh"

#include "reduced_costs.hh"
#include "robust_utils.hh"

SearchingRobustRouter::SearchingRobustRouter(const Graph& graph,
                                             const EdgeFunc<num>& costs,
                                             const EdgeFunc<num>& deviations,
                                             idx deviationSize,
                                             ThetaRouter& router,
                                             Options options)
  : RobustRouter(graph, costs, deviations, deviationSize),
    router(router),
    options(options),
    intervalSelection(LOWEST_BOUND)
{

}

RobustSearchResult SearchingRobustRouter::shortestPath(Vertex source,
                                                       Vertex target,
                                                       const ValueVector& possibleValues,
                                                       num bound)
{
  Path bestPath;
  num bestCost = inf;
  bool found = false;
  RobustSearchResult robustSearchResult;

  auto evaluate = [&] (num value) -> num
    {
      const num costBound = getBound(bound, bestCost, value);

      SearchResult result = router.shortestPath(source,
                                                target,
                                                value,
                                                costBound);

      robustSearchResult.add(result);

      num pathCost = inf;
      num cost = inf;

      assert(verifyResult(result, source, target, value, costBound));

      if(result.found)
      {
        pathCost = result.cost;
        cost = deviationSize*value + pathCost;
        found = true;

        if(cost < bestCost)
        {
          bestCost = cost;
          bestPath = result.path;
        }
      }

      return pathCost;
    };

  typedef typename boost::heap::d_ary_heap<SearchInterval,
                                           boost::heap::compare<SearchIntervalCompare>,
                                           boost::heap::arity<2>> SearchIntervalQueue;

  SearchIntervalCompare intervalCompare(intervalSelection);
  SearchIntervalQueue intervals(intervalCompare);

  num leftValue = *(possibleValues.begin());
  num leftPathCost = evaluate(leftValue);

  num rightValue = *(possibleValues.rbegin());
  num rightPathCost = evaluate(rightValue);

  intervals.push(SearchInterval(possibleValues, leftPathCost, rightPathCost, deviationSize));

  while(!intervals.empty())
  {
    SearchInterval interval = intervals.top();

    intervals.pop();
    const int size = interval.getValues().size();

    if(size <= 2)
    {
      continue;
    }

    if(interval.lowerBoundCost >= bestCost)
    {
      continue;
    }

    if(options & TIGHTENING)
    {
      if(interval.tighten(bestCost, deviationSize))
      {
        if(interval.getValues().size() <= 1)
        {
          continue;
        }

        interval.rightPathCost = evaluate(interval.getValues().last());
      }
    }

    auto middle = interval.getValues().middle();

    num pathCost = evaluate(*middle);

    intervals.push(SearchInterval::leftInterval(interval, middle, pathCost));
    intervals.push(SearchInterval::rightInterval(interval, middle, pathCost));

  }

  if(found)
  {
    robustSearchResult.found = true;
    robustSearchResult.path = bestPath;
  }

  return robustSearchResult;
}

bool SearchingRobustRouter::verifyResult(const SearchResult& result,
                                         Vertex source,
                                         Vertex target,
                                         num value,
                                         num costBound)
{
  BidirectionalDijkstra router(graph);
  ReducedCosts reducedCosts(costs, deviations, value);
  auto simpleResult = router.shortestPath(source,
                                          target,
                                          reducedCosts,
                                          costBound);

  if(simpleResult.found != result.found)
  {
    return false;
  }

  if(result.found)
  {
    if(result.cost > costBound)
    {
      return false;
    }
    if(!result.path.connects(source, target))
    {
      return false;
    }

    if(!simpleResult.path.connects(source, target))
    {
      return false;
    }

    if(simpleResult.cost != result.cost)
    {
      return false;
    }
  }

  return true;
}

num SearchingRobustRouter::getBound(num bound, num bestCost, num value)
{
  return (options & BOUNDING) ?
    std::min(bound, bestCost - ((num) deviationSize) * value) :
    bound;
}

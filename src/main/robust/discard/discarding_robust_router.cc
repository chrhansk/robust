#include "discarding_robust_router.hh"

#include "log.hh"

#include "robust/arcflags/arcflag_theta_router.hh"

DiscardingRobustRouter::DiscardingRobustRouter(const Graph& graph,
                                               const EdgeFunc<num>& costs,
                                               const EdgeFunc<num>& deviations,
                                               idx deviationSize,
                                               ThetaRouter& router,
                                               const DiscardingPreprocessor& preprocessor,
                                               SearchingRobustRouter::Options options)
  : SearchingRobustRouter(graph, costs, deviations, deviationSize, router, options),
    preprocessor(preprocessor)
{
  ValueVector nextValues;

  const ValueSet& bestValues = preprocessor.getBestValues();

  for(const num& value : values)
  {
    if(bestValues.find(value) == bestValues.end())
    {
      nextValues.push_back(value);
    }
  }

  values = nextValues;
}

RobustSearchResult DiscardingRobustRouter::shortestPath(Vertex source,
                                                        Vertex target,
                                                        const ValueVector& possibleValues,
                                                        num bound)
{
  Path bestPath;
  num bestCost = inf;
  bool found = false;
  RobustSearchResult robustSearchResult;

  auto evaluate = [&] (ThetaRouter& router, num value) -> num
    {
      const num costBound = inf;
      //const num costBound = getBound(bound, bestCost, value);

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

  {
    ArcFlagThetaRouter<SimpleArcFlags> arcFlagRouter(graph, costs, deviations,
                                                     preprocessor.getPartition(),
                                                     preprocessor.getArcFlags());

    for(const num& value : preprocessor.getBestValues())
    {
      evaluate(arcFlagRouter, value);
    }
  }

  typedef typename boost::heap::d_ary_heap<SearchInterval,
                                           boost::heap::compare<SearchIntervalCompare>,
                                           boost::heap::arity<2>> SearchIntervalQueue;

  SearchIntervalCompare intervalCompare(intervalSelection);
  SearchIntervalQueue intervals(intervalCompare);

  num rightValue = *(possibleValues.rbegin());
  num rightPathCost = evaluate(router, rightValue);

  ValueInterval allValues(possibleValues);

  auto discardValue = [&](num value) -> bool {
    return preprocessor.canDiscard(source, target, value, deviationSize, bestCost);
  };

  allValues.trimBegin(discardValue);

  Log(info) << "Initial values: " << allValues.size()
            << " (decreased from " << possibleValues.size() << ")";

  num leftValue = *(allValues.begin());
  num leftPathCost = evaluate(router, leftValue);

  intervals.push(SearchInterval(allValues, leftPathCost, rightPathCost, deviationSize));

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

        interval.rightPathCost = evaluate(router, interval.getValues().last());
      }
    }

    idx initialSize = interval.getValues().size();

    if(interval.getValues().trimBegin(discardValue))
    {
      if(interval.getValues().size() <= 1)
      {
        continue;
      }

      interval.leftPathCost = evaluate(router, interval.getValues().first());
    }

    if(interval.getValues().trimEnd(discardValue))
    {
      if(interval.getValues().size() <= 1)
      {
        continue;
      }

      interval.rightPathCost = evaluate(router, interval.getValues().last());
    }

    if(interval.getValues().size() < initialSize)
    {
      Log(info) << "Size decreased from " << initialSize
                << " to " << interval.getValues().size();
    }

    auto middle = interval.getValues().middle();

    num pathCost = evaluate(router, *middle);

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

#include "active_router.hh"

#include "robust/reduced_costs.hh"

namespace
{

  class PathCost
  {
  private:
    num value;
    num cost;

  public:
    PathCost(num value, num cost)
      : value(value),
        cost(cost)
    {}

    num getValue() const
    {
      return value;
    }

    num getCost() const
    {
      return cost;
    }

    bool operator==(const PathCost& other) const
    {
      return getValue() == other.getValue() and
        getCost() == other.getCost();
    }
  };

  class ActiveEdgeSet
  {
  private:
    const EdgeFunc<num>& deviations;
    std::vector<Edge> edges;
    idx numHeavy;
    PathCost leftCost, rightCost;

  public:
    ActiveEdgeSet(const EdgeFunc<num>& deviations,
                  const std::vector<Edge>& edges,
                  PathCost leftCost,
                  PathCost rightCost);

    ActiveEdgeSet(const ActiveEdgeSet& other,
                  PathCost leftCost);

    num getLowerBound(const num value) const;

    const EdgeFunc<num>& getDeviations() const
    {
      return deviations;
    }

    const std::vector<Edge>& getEdges() const
    {
      return edges;
    }
  };


  ActiveEdgeSet::ActiveEdgeSet(const EdgeFunc<num>& deviations,
                               const std::vector<Edge>& otherEdges,
                               PathCost leftCost,
                               PathCost rightCost)
    : deviations(deviations),
      numHeavy(0),
      leftCost(leftCost),
      rightCost(rightCost)
  {
    const num minValue = rightCost.getValue();
    const num maxValue = leftCost.getValue();

    assert(minValue < maxValue);
    assert(leftCost.getCost() <= rightCost.getCost());

    for(const Edge& edge : otherEdges)
    {
      assert(deviations(edge) > minValue);

      if(deviations(edge) >= maxValue)
      {
        ++numHeavy;
      }
      else
      {
        edges.push_back(edge);
      }
    }

    std::sort(edges.begin(), edges.end(),
              [&](const Edge& first, const Edge& second)
              {
                return deviations(first) < deviations(second);
              });
  }

  ActiveEdgeSet::ActiveEdgeSet(const ActiveEdgeSet& other,
                               PathCost leftCost)
    : deviations(other.getDeviations()),
      numHeavy(other.numHeavy),
      leftCost(leftCost),
      rightCost(other.rightCost)
  {
    const num minValue = rightCost.getValue();
    const num maxValue = leftCost.getValue();

    assert(leftCost.getValue() <= other.leftCost.getValue());
    assert(leftCost.getCost() >= other.leftCost.getCost());
    assert(minValue < maxValue);
    assert(leftCost.getCost() <= rightCost.getCost());

    for(const Edge& edge : other.edges)
    {
      assert(deviations(edge) > minValue);

      if(deviations(edge) >= maxValue)
      {
        ++numHeavy;
      }
      else
      {
        edges.push_back(edge);
      }
    }
  }

  num ActiveEdgeSet::getLowerBound(const num value) const
  {
    const num capacity = rightCost.getCost() - leftCost.getCost();

    assert(capacity >= 0);

    const num minValue = rightCost.getValue();
    const num maxValue = leftCost.getValue();

    assert(value <= maxValue);
    assert(value > minValue);

    float totalWeight = 0;
    float totalGain = 0;

    auto gains = [&](const Edge& edge) -> num
      {
        return (std::min(deviations(edge), value) - minValue);
      };

    auto weights = [&](const Edge& edge) -> num
      {
        return (std::min(deviations(edge), maxValue) - minValue);
      };

    /*
    auto benefits = [&](const Edge& edge) -> float
      {
        return gains(edge) / ((float) weights(edge));
      };
    */

    auto it = edges.begin();

    for(; it != edges.end(); ++it)
    {
      const Edge& edge = *it;
      const num deviation = deviations(edge);

      assert(totalWeight <= capacity);

      if(deviation <= value)
      {
        assert(weights(edge) <= maxValue);
        assert(gains(edge) == deviation - minValue);
        assert(weights(edge) == deviation - minValue);

        const num remainingWeight = capacity - totalWeight;
        const num currentWeight = deviation - minValue;

        if(currentWeight <= remainingWeight)
        {
          totalGain += currentWeight;
          totalWeight += currentWeight;
        }
        else
        {
          float weightFraction = remainingWeight / ((float) currentWeight);
          assert(weightFraction < 1);

          float weight = weightFraction * currentWeight;

          totalWeight += weight;
          totalGain += weight;

          return rightCost.getCost() - totalGain;
        }
      }
      else
      {
        break;
      }
    }

    std::vector<Edge> nextEdges;

    for(; it != edges.end(); ++it)
    {
        if(debuggingEnabled())
        {
        num deviation = deviations(*it);
        assert(deviation > value);
        }
      nextEdges.push_back(*it);
    }

    for(const Edge& edge : nextEdges)
    {
      assert(totalWeight <= capacity);
      const num currentWeight = weights(edge);
      const num currentGain = gains(edge);

      assert(currentGain < currentWeight);

      if(totalWeight + currentWeight <= capacity)
      {
        totalWeight += currentWeight;
        totalGain += currentGain;
      }
      else
      {
        num remainingWeight = capacity - totalWeight;
        float weightFraction = remainingWeight / ((float) currentWeight);

        assert(weightFraction < 1);

        totalWeight += weightFraction;
        totalGain += weightFraction * currentGain;
        break;
      }
    }


    if(numHeavy > 0 and totalWeight < capacity)
    {
      num remainingWeight = capacity - totalWeight;
      float currentWeight = maxValue - minValue;
      num currentGain = (value - minValue);
      num gain = std::min((float) numHeavy,
                          remainingWeight / currentWeight) * currentGain;

      assert(currentGain < currentWeight);

      totalGain += gain;
    }

    float boundValue = rightCost.getCost() - totalGain;

    return boundValue;
  }

  class ActiveInterval
  {
  private:
    void check() const;
    // The given range does *not* include leftValue / rightValue
    ValueRange range;
    const ActiveEdgeSet activeEdges;

  public:
    ActiveInterval(ValueRange range,
                   PathCost leftPathCost,
                   PathCost rightPathCost,
                   const ActiveEdgeSet& activeEdges);

    ActiveInterval(const ValueVector& values,
                   const EdgeFunc<num>& deviations,
                   PathCost leftPathCost,
                   PathCost rightPathCost,
                   const std::vector<Edge>& activeEdges);

    PathCost leftPathCost, rightPathCost;

    const ActiveEdgeSet& getActiveEdges() const
    {
      return activeEdges;
    }

    ValueRange getRange() const
    {
      return range;
    }

    float getLowerBound(num value) const
    {
      return activeEdges.getLowerBound(value);
    }

    std::pair<ActiveInterval, ActiveInterval>
    split(ValueIterator middle,
          num value,
          const std::vector<Edge>& activeEdges);
  };



  ActiveInterval::ActiveInterval(ValueRange range,
                                 PathCost leftPathCost,
                                 PathCost rightPathCost,
                                 const ActiveEdgeSet& activeEdges)
    : range(range),
      activeEdges(activeEdges),
      leftPathCost(leftPathCost),
      rightPathCost(rightPathCost)
  {
    check();
  }

  ActiveInterval::ActiveInterval(const ValueVector& values,
                                 const EdgeFunc<num>& deviations,
                                 PathCost leftPathCost,
                                 PathCost rightPathCost,
                                 const std::vector<Edge>& edges)
    : range(ValueRange(values).innerRange()),
      activeEdges(deviations,
                  edges,
                  leftPathCost,
                  rightPathCost),
      leftPathCost(leftPathCost),
      rightPathCost(rightPathCost)
  {
    check();
  }

  void ActiveInterval::check() const
  {
    assert(leftPathCost.getValue() > *(range.begin()));
    assert(rightPathCost.getValue() < *(range.rbegin()));
    assert(leftPathCost.getCost() <= rightPathCost.getCost());
  }

  std::pair<ActiveInterval, ActiveInterval>
  ActiveInterval::split(ValueIterator middle,
                        num cost,
                        const std::vector<Edge>& nextEdges)
  {
    num value = *middle;
    auto ranges = range.split(middle);

    PathCost pathCost(value, cost);

    return std::make_pair(
      ActiveInterval(ranges.first,
                     leftPathCost,
                     pathCost,
                     ActiveEdgeSet(activeEdges.getDeviations(),
                                   nextEdges,
                                   leftPathCost,
                                   pathCost)),

      ActiveInterval(ranges.second,
                     pathCost,
                     rightPathCost,
                     ActiveEdgeSet(activeEdges,
                                   pathCost)));
  }

}

ActiveRouter::ActiveRouter(const Graph& graph,
                           const EdgeFunc<num>& costs,
                           const EdgeFunc<num>& deviations,
                           idx deviationSize)
  : RobustRouter(graph, costs, deviations, deviationSize)
{

}

num ActiveRouter::getPathCost(const Path& path, num value)
{
  return path.cost(ReducedCosts(costs, deviations, value));
}

RobustSearchResult ActiveRouter::shortestPath(Vertex source,
                                              Vertex target,
                                              const ValueVector& possibleValues,
                                              num bound)
{
  Path bestPath;
  num bestCost = inf;
  RobustSearchResult robustSearchResult;

  auto evaluateResult = [&](ActiveSearchResult& activeResult,
                           num value)
    {
      robustSearchResult.add(activeResult);

      const num pathCost = getPathCost(activeResult.path, value);
      const num cost = deviationSize*value + pathCost;

      if(cost < bestCost)
      {
        bestCost = cost;
        bestPath = activeResult.path;
      }
    };

  auto evaluate = [&] (num value) -> ActiveSearchResult
    {
      /*
        std::cout << "Evaluating at " << value << std::endl;
      */

      ActiveSearchResult activeResult = findShortestPath(source,
                                                         target,
                                                         value);

      evaluateResult(activeResult, value);

      return activeResult;
    };

  std::queue<ActiveInterval> intervals;

  num leftValue = *(possibleValues.begin());

  auto leftResult = findMaxShortestPath(source,target, leftValue);
  evaluateResult(leftResult, leftValue);
  PathCost leftPathCost(leftValue, getPathCost(leftResult.path, leftValue));

  num rightValue = *(possibleValues.rbegin());

  auto rightResult = evaluate(rightValue);
  PathCost rightPathCost(rightValue, getPathCost(rightResult.path, rightValue));

  ActiveInterval interval(possibleValues,
                          deviations,
                          leftPathCost,
                          rightPathCost,
                          rightResult.getActiveEdges());

  intervals.push(interval);

  while(!intervals.empty())
  {
    ActiveInterval interval = intervals.front();
    intervals.pop();

    if(interval.leftPathCost == interval.rightPathCost)
    {
      continue;
    }

    const idx size = interval.getRange().getSize();

    if(size <= 2)
    {
      continue;
    }

    auto intervalDiscard = [&](num value)
      {
        float bound = interval.getLowerBound(value);

        if(debuggingEnabled())
        {
          Dijkstra dijkstra(graph);
          ReducedCosts reducedCosts(costs, deviations, value);

          auto result = dijkstra.shortestPath(source, target, reducedCosts);

          assert(result.path.cost(reducedCosts) >= bound);
        }

        float costBound = bound + value*deviationSize;

        return costBound >= bestCost;
      };

    auto middle = interval.getRange().middle();

    if(intervalDiscard(*middle))
    {
      auto pair = interval.getRange().split(middle);

      {
        num leftValue = *middle;
        ValueRange range = pair.second;

        assert(middle < range.begin());

        ValueIterator it = range.begin();

        while(it != range.end() and intervalDiscard(*it))
        {
          leftValue = *it;
          ++it;
        }

        if(it != range.end())
        {
          assert(!intervalDiscard(*it));

          range = ValueRange(it, range.end());

          auto result = evaluate(leftValue);
          PathCost leftPathCost(leftValue, getPathCost(result.path, leftValue));

          intervals.push(ActiveInterval(range,
                                        leftPathCost,
                                        interval.rightPathCost,
                                        interval.getActiveEdges()));
        }

        {
          num rightValue = *middle;
          ValueRange range = pair.first;

          ReverseValueIterator rit = range.rbegin();

          while(rit != range.rend() and intervalDiscard(*rit))
          {
            rightValue = *rit;
            ++rit;
          }

          if(rit != range.rend())
          {
            assert(!intervalDiscard(*rit));

            range = ValueRange(rit, range.rend());

            auto result = evaluate(rightValue);
            PathCost rightPathCost(rightValue, getPathCost(result.path, rightValue));

            intervals.push(ActiveInterval(range,
                                          interval.leftPathCost,
                                          rightPathCost,
                                          ActiveEdgeSet(deviations,
                                                        result.getActiveEdges(),
                                                        interval.leftPathCost,
                                                        rightPathCost)));
          }

        }

      }
    }
    else
    {
      auto result = evaluate(*middle);
      num pathCost = getPathCost(result.path, *middle);

      auto currentPair = interval.split(middle, pathCost, result.getActiveEdges());

      const ActiveInterval& leftInterval = currentPair.first;

      const ActiveInterval& rightInterval = currentPair.second;

      intervals.push(leftInterval);

      intervals.push(rightInterval);
    }
  }

  robustSearchResult.found = true;
  robustSearchResult.path = bestPath;

  return robustSearchResult;
}


/*
ActiveRouter::ActiveSearchResult
ActiveRouter::findShortestPath(Vertex source,
                                   Vertex target,
                                   num value)
{
  auto reducedCosts = ReducedCosts(costs, deviations, value);

  LabelHeap<Label> heap(graph);
  int settled = 0, labeled = 0;
  bool found = false;

  heap.update(Label(source, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    ++settled;

    if(current.getVertex() == target)
    {
      found = true;
      break;
    }

    for(const Edge& edge : graph.getOutgoing(current.getVertex()))
    {
      ++labeled;

      Label nextLabel = Label(edge.getTarget(),
                              edge, current.getCost() + reducedCosts(edge));

      heap.update(nextLabel);
    }
  }

  if(found)
  {
    Path path;

    Label current = heap.getLabel(target);

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      path.prepend(edge);
      current = heap.getLabel(edge.getSource());
    }

    ActiveSearchResult searchResult(SearchResult(settled, labeled, found, path));

    for(const Edge& edge : graph.getEdges())
    {
      if(heap.getLabel(edge.getSource()).getState() == State::SETTLED)
      {
        if(deviations(edge) > value)
        {
          searchResult.getActiveEdges().push_back(edge);
        }
      }
    }
  }

  return ActiveSearchResult(SearchResult(settled, labeled, false, Path()));
}
*/

#include "bidirectional_goal_directed_router.hh"

#include "log.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"

const BidirectionalGoalDirectedRouter::PotentialCombination
BidirectionalGoalDirectedRouter::Average = [](num first, num second) {return (first + second) / ((num) 2);};

const BidirectionalGoalDirectedRouter::PotentialCombination
BidirectionalGoalDirectedRouter::Max = [](num first, num second) {return std::max(first, second);};

const BidirectionalGoalDirectedRouter::PotentialCombination
BidirectionalGoalDirectedRouter::Min = [](num first, num second) {return std::min(first, second);};

BidirectionalGoalDirectedRouter::BidirectionalGoalDirectedRouter(const Graph& graph,
                                                                 const EdgeFunc<num>& costs,
                                                                 const EdgeFunc<num>& deviations,
                                                                 idx deviationsize,
                                                                 const PotentialCombination& combination,
                                                                 bool extendedRecomputation)
  : BidirectionalRouter(graph),
    costs(costs),
    deviations(deviations),
    potentialCombination(combination),
    forwardDistances(graph),
    backwardDistances(graph),
    shouldRecompute(true),
    extendedRecomputation(extendedRecomputation)
{
}

void BidirectionalGoalDirectedRouter::doReset()
{
  forwardDistances.reset();
  backwardDistances.reset();
}

SearchResult BidirectionalGoalDirectedRouter::shortestPath(Vertex source,
                                                           Vertex target,
                                                           num thetaValue,
                                                           num boundValue)
{
  return findShortestPath<true>(source, target, thetaValue, boundValue);
}

SearchResult BidirectionalGoalDirectedRouter::shortestPath(Vertex source,
                                                           Vertex target,
                                                           num thetaValue)
{
  return findShortestPath<false>(source, target, thetaValue, inf);
}

template <bool bounded>
SearchResult BidirectionalGoalDirectedRouter::findShortestPath(Vertex source,
                                                               Vertex target,
                                                               num thetaValue,
                                                               num boundValue)
{
  if(reset(source, target, thetaValue))
  {
    auto result = recomputePotential<bounded>(source,
                                              target,
                                              thetaValue,
                                              boundValue);

    lastSettled = result.settled;
    shouldRecompute = false;

    return result;
  }
  else if(shouldRecompute)
  {
    doReset();

    auto result = recomputePotential<bounded>(source,
                                              target,
                                              thetaValue,
                                              boundValue);

    lastSettled = result.settled;
    shouldRecompute = false;

    return result;
  }
  else
  {
    auto result = computeShortestPath<bounded>(source,
                                               target,
                                               thetaValue,
                                               boundValue);

    if(result.settled >= recomputationFactor * lastSettled)
    {
      //Log(debug) << "Performing recomputation next time";
      shouldRecompute = true;
    }

    return result;
  }

}


template<bool bounded>
SearchResult BidirectionalGoalDirectedRouter::recomputePotential(Vertex source,
                                                                 Vertex target,
                                                                 num thetaValue,
                                                                 num boundValue)
{
  int settled = 0, labeled = 0;
  bool found = false;

  Vertex split;
  num splitValue = inf;

  ReducedCosts reducedCosts(costs, deviations, thetaValue);

  LabelHeap<Label> forwardHeap(graph);
  LabelHeap<Label> backwardHeap(graph);

  forwardHeap.update(Label(source, Edge(), 0));
  backwardHeap.update(Label(target, Edge(), 0));

  bool sourceReached = false, targetReached = false;

  forwardDistances.setDefaultValue(0);
  backwardDistances.setDefaultValue(0);

  while(!(forwardHeap.isEmpty() or backwardHeap.isEmpty()))
  {
    const num bestValue = forwardHeap.peek().getCost()
      + backwardHeap.peek().getCost();

    if(extendedRecomputation)
    {
      if(sourceReached and targetReached)
      {
        break;
      }
    }
    else
    {
      if(bestValue >= splitValue)
      {
        break;
      }

      if(bounded and
         bestValue > boundValue)
      {
        break;
      }
    }

    if(forwardHeap.peek().getCost() < backwardHeap.peek().getCost() and
       !targetReached)
    {
      Label current = forwardHeap.extractMin();

      forwardDistances.setValue(current.getVertex(), current.getCost());
      forwardDistances.setDefaultValue(current.getCost());

      if(current.getVertex() == target)
      {
        targetReached = true;
      }

      ++settled;

      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        Vertex nextVertex = edge.getTarget();
        num nextCost = current.getCost() + reducedCosts(edge);
        ++labeled;

        forwardHeap.update(Label(nextVertex, edge, nextCost));

        const Label& other = backwardHeap.getLabel(nextVertex);

        if(other.getState() != State::UNKNOWN)
        {
          num value = other.getCost() + nextCost;

          if(value < splitValue)
          {
            splitValue = value;
            split = nextVertex;
            found = true;
          }
        }
      }
    }
    else
    {
      Label current = backwardHeap.extractMin();

      backwardDistances.setValue(current.getVertex(), current.getCost());
      backwardDistances.setDefaultValue(current.getCost());

      if(current.getVertex() == source)
      {
        sourceReached = true;
      }

      ++settled;

      for(const Edge& edge : graph.getIncoming(current.getVertex()))
      {
        Vertex nextVertex = edge.getSource();
        num nextCost = current.getCost() + reducedCosts(edge);

        ++labeled;

        backwardHeap.update(Label(nextVertex, edge, nextCost));

        const Label& other = forwardHeap.getLabel(nextVertex);

        if(other.getState() != State::UNKNOWN)
        {
          num value = other.getCost() + nextCost;

          if(value < splitValue)
          {
            splitValue = value;
            split = nextVertex;
            found = true;
          }
        }
      }
    }
  }

  /*
  Log(debug) << "Recomputing the potential for "
             << thetaValue
             << " settled " << settled
             << " vertices";
  */

  if(found)
  {
    if(bounded)
    {
      if(splitValue > boundValue)
      {
        return SearchResult::notFound(settled, labeled);
      }
    }

    Path path;

    Label current = forwardHeap.getLabel(split);

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      path.prepend(edge);
      current = forwardHeap.getLabel(edge.getSource());
    }

    current = backwardHeap.getLabel(split);

    while(!(current.getVertex() == target))
    {
      Edge edge = current.getEdge();
      path.append(edge);
      current = backwardHeap.getLabel(edge.getTarget());
    }

    assert(path.connects(source, target));
    assert(path.cost(reducedCosts) == splitValue);

    return SearchResult(settled, labeled, true, path, splitValue);
  }

  return SearchResult::notFound(settled, labeled);
}

template<bool bounded>
SearchResult BidirectionalGoalDirectedRouter::computeShortestPath(Vertex source,
                                                                  Vertex target,
                                                                  num theta,
                                                                  num bound)
{
  ReducedCosts reducedCosts(costs, deviations, theta);

  CombinedVertexFunc<num, PotentialCombination> combinedFunction(potentialCombination,
                                                                 forwardDistances,
                                                                 backwardDistances);

  SimplePotential combinedPotential(graph, combinedFunction);

  const num potentialCostBound = combinedPotential.potentialPathCost(bound,
                                                                     source,
                                                                     target);

  assert(combinedPotential.isValidFor(reducedCosts));

  auto result = BidirectionalRouter::shortestPath<AllEdgeFilter,
                                                  AllEdgeFilter,
                                                  bounded>(source,
                                                           target,
                                                           PotentialCosts(reducedCosts, combinedPotential),
                                                           AllEdgeFilter(),
                                                           AllEdgeFilter(),
                                                           potentialCostBound);

  if(result.found)
  {
    result.cost = combinedPotential.actualPathCost(result.cost, source, target);
  }

  /*
  Log(debug) << "Computing the shortest path for "
             << theta
             << " using the potential settled " << result.settled
             << " vertices";
  */

  return result;
}

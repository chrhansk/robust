#include "goal_directed_router.hh"

#include "log.hh"

#include "robust/reduced_costs.hh"

GoalDirectedRouter::GoalDirectedRouter(const Graph& graph,
                                       const EdgeFunc<num>& costs,
                                       const EdgeFunc<num>& deviations,
                                       idx deviationSize)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    partialDistances(graph),
    shouldRecompute(true)
{

}

template <bool bounded>
SearchResult GoalDirectedRouter::findShortestPath(Vertex source,
                                                  Vertex target,
                                                  num thetaValue,
                                                  num boundValue)
{
  if(source == target)
  {
    return SearchResult(0, 0, true, Path(), 0);
  }

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
      //Log(info) << "Reset due to increased exploration";
      shouldRecompute = true;
    }

    return result;
  }
}

SearchResult GoalDirectedRouter::shortestPath(Vertex source,
                                              Vertex target,
                                              num theta,
                                              num bound)
{
  return findShortestPath<true>(source, target, theta, bound);
}

SearchResult GoalDirectedRouter::shortestPath(Vertex source,
                                              Vertex target,
                                              num theta)
{
  return findShortestPath<false>(source, target, theta, inf);
}

void GoalDirectedRouter::doReset()
{
  partialDistances.reset();
}


template<bool bounded>
SearchResult
GoalDirectedRouter::recomputePotential(Vertex source,
                                       Vertex target,
                                       num theta,
                                       num bound)
{
  LabelHeap<Label> heap(graph);
  int settled = 0, labeled = 0;
  bool found = false;

  ReducedCosts reducedCosts(costs, deviations, theta);

  partialDistances.setDefaultValue(0);

  heap.update(Label(target, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    ++settled;

    partialDistances.setValue(current.getVertex(), current.getCost());
    partialDistances.setDefaultValue(current.getCost());

    if(bounded)
    {
      if(current.getCost() > bound)
      {
        break;
      }
    }

    if(current.getVertex() == source)
    {
      found = true;
      break;
    }

    for(const Edge& edge : graph.getIncoming(current.getVertex()))
    {
      ++labeled;

      Label nextLabel = Label(edge.getSource(),
                              edge, current.getCost() + reducedCosts(edge));

      heap.update(nextLabel);
    }

  }

  /*
  Log(debug) << "Recomputing the potential for "
             << theta
             << " settled " << settled
             << " vertices";
  */

  if(found)
  {
    Path path;

    Label current = heap.getLabel(source);
    const num cost = current.getCost();

    while(current.getVertex() != target)
    {
      Edge edge = current.getEdge();
      path.append(edge);
      current = heap.getLabel(edge.getTarget());
    }

    return SearchResult(settled, labeled, true, path, cost);
  }

  return SearchResult::notFound(settled, labeled);
}


template<bool bounded>
SearchResult
GoalDirectedRouter::computeShortestPath(Vertex source,
                                        Vertex target,
                                        num theta,
                                        num bound) const
{
  Dijkstra dijkstra(graph);

  ReducedCosts reducedCosts(costs, deviations, theta);
  SimplePotential potential(graph, partialDistances);
  PotentialCosts potentialCosts(reducedCosts, potential);

  assert(potential.isValidFor(reducedCosts));

  const num potentialCostBound = potential.potentialPathCost(bound, source, target);

  auto result = dijkstra.shortestPath<AllEdgeFilter, bounded>(source,
                                                              target,
                                                              potentialCosts,
                                                              AllEdgeFilter(),
                                                              potentialCostBound);

  if(result.found)
  {
    result.cost = potential.actualPathCost(result.cost, source, target);
  }

  /*
  Log(debug) << "Computing the shortest path for "
             << theta
             << " using the potential settled " << result.settled
             << " vertices";
  */

  return result;
}

#include "goal_directed_bounding_router.hh"

#include "robust/reduced_costs.hh"

GoalDirectedBoundingRouter::GoalDirectedBoundingRouter(const Graph& graph,
                                                       const EdgeFunc<num>& costs,
                                                       const EdgeFunc<num>& deviations,
                                                       idx deviationSize)
  : GoalDirectedRouter(graph, costs, deviations, deviationSize),
    deviationSize(deviationSize),
    upperBounds(graph, inf)
{

}

template <bool bounded>
SearchResult GoalDirectedBoundingRouter::findShortestPath(Vertex source,
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

SearchResult GoalDirectedBoundingRouter::shortestPath(Vertex source,
                                                      Vertex target,
                                                      num theta,
                                                      num bound)
{
  return findShortestPath<true>(source, target, theta, bound);
}

SearchResult GoalDirectedBoundingRouter::shortestPath(Vertex source,
                                                      Vertex target,
                                                      num theta)
{
  return findShortestPath<false>(source, target, theta, inf);
}

template<bool bounded>
SearchResult GoalDirectedBoundingRouter::computeShortestPath(Vertex source,
                                                             Vertex target,
                                                             num thetaValue,
                                                             num boundValue)
{
  ReducedCosts reducedCosts(costs, deviations, thetaValue);
  SimplePotential potential(graph, partialDistances);
  PotentialCosts potentialCosts(reducedCosts, potential);

  LabelHeap<Label> heap(graph);
  int settled = 0, labeled = 0;
  bool found = false;

  heap.update(Label(source, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    const Vertex currentVertex = current.getVertex();
    const num currentPotentialCost = current.getCost();
    const num currentActualCost = potential.actualPathCost(currentPotentialCost,
                                                           source,
                                                           target);

    ++settled;

    if(bounded)
    {
      if(currentActualCost > boundValue)
      {
        break;
      }
    }

    if(currentVertex == target)
    {
      found = true;
      break;
    }

    for(const Edge& edge : graph.getOutgoing(currentVertex))
    {
      ++labeled;

      const Vertex nextVertex = edge.getTarget();

      const num nextPotentialCost = currentPotentialCost + potentialCosts(edge);
      const num nextActualCost = potential.actualPathCost(nextPotentialCost,
                                                          source,
                                                          nextVertex);

      Label nextLabel = Label(edge.getTarget(),
                              edge, nextPotentialCost);

      num costBound = deviationSize * thetaValue + nextActualCost;

      if(costBound > upperBounds(nextVertex))
      {
        continue;
      }
      else
      {
        upperBounds(nextVertex) = costBound;
      }

      heap.update(nextLabel);
    }

  }

  if(found)
  {
    Path path;

    Label current = heap.getLabel(target);
    const num cost = current.getCost();

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      path.prepend(edge);
      current = heap.getLabel(edge.getSource());
    }

    return SearchResult(settled, labeled, true, path,
                        potential.actualPathCost(cost, source, target));
  }

  return SearchResult::notFound(settled, labeled);
}


void GoalDirectedBoundingRouter::doReset()
{
  GoalDirectedRouter::doReset();
  upperBounds.reset(inf);
}

#include "goal_directed_active_router.hh"

#include "log.hh"

#include "robust/reduced_costs.hh"

ActiveRouter::ActiveSearchResult
GoalDirectedActiveRouter::findShortestPath(Vertex source,
                                           Vertex target,
                                           num value)
{

  ReducedCosts reducedCosts(costs, deviations, value);
  SimplePotential potential(graph, partialDistances);
  PotentialCosts potentialCosts(reducedCosts, potential);

  assert(potential.isValidFor(reducedCosts));

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
                              edge, current.getCost() + potentialCosts(edge));

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

    auto result = ActiveSearchResult(SearchResult(settled,
                                                  labeled,
                                                  true,
                                                  path,
                                                  potential.actualPathCost(current.getCost(),
                                                                           source,
                                                                           target)));

    for(const Edge& edge : graph.getEdges())
    {
      if(heap.getLabel(edge.getSource()).getState() == State::SETTLED)
      {
        if(deviations(edge) > value)
        {
          result.getActiveEdges().push_back(edge);
        }
      }
    }

    return result;
  }

  return ActiveSearchResult(SearchResult::notFound(settled, labeled));

}

ActiveRouter::ActiveSearchResult
GoalDirectedActiveRouter::findMaxShortestPath(Vertex source,
                                              Vertex target,
                                              num value)
{
  doReset();

  SearchResult result = recomputePotential<false>(source,
                                                  target,
                                                  value,
                                                  inf);

  return ActiveSearchResult(result);
}

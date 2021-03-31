#include "bounding_router.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "robust/reduced_costs.hh"

BoundingRouter::BoundingRouter(const Graph& graph,
                               const EdgeFunc<num>& costs,
                               const EdgeFunc<num>& deviations,
                               idx deviationSize)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    deviationSize(deviationSize),
    upperBounds(graph, inf)
{
}


void BoundingRouter::doReset()
{
  upperBounds.reset(inf);
}

template <bool bounded>
SearchResult BoundingRouter::findShortestPath(Vertex source,
                                              Vertex target,
                                              num thetaValue,
                                              num boundValue)
{
  reset(source, target, thetaValue);

  ReducedCosts reducedCosts(costs, deviations, thetaValue);

  LabelHeap<Label> heap(graph);
  int settled = 0, labeled = 0;
  bool found = false;

  heap.update(Label(source, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    ++settled;

    if(bounded)
    {
      if(current.getCost() > boundValue)
      {
        break;
      }
    }

    if(current.getVertex() == target)
    {
      found = true;
      break;
    }

    for(const Edge& edge : graph.getOutgoing(current.getVertex()))
    {
      ++labeled;

      const num nextCost = current.getCost() + reducedCosts(edge);

      Label nextLabel = Label(edge.getTarget(),
                              edge, nextCost);

      const Vertex nextVertex = edge.getTarget();
      num costBound = deviationSize * thetaValue + nextCost;

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

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      path.prepend(edge);
      current = heap.getLabel(edge.getSource());
    }

    return SearchResult(settled, labeled, true, path, current.getCost());
  }

  return SearchResult::notFound(settled, labeled);

}

SearchResult BoundingRouter::shortestPath(Vertex source,
                                          Vertex target,
                                          num thetaValue,
                                          num bound)
{
  return findShortestPath<true>(source, target, thetaValue, bound);
}

SearchResult BoundingRouter::shortestPath(Vertex source,
                                          Vertex target,
                                          num thetaValue)
{
  return findShortestPath<false>(source, target, thetaValue, inf);
}

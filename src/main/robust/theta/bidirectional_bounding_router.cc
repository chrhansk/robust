#include "bidirectional_bounding_router.hh"

#include "robust/reduced_costs.hh"

BidirectionalBoundingRouter::BidirectionalBoundingRouter(const Graph& graph,
                                                         const EdgeFunc<num>& costs,
                                                         const EdgeFunc<num>& deviations,
                                                         idx deviationSize)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    deviationSize(deviationSize),
    forwardBounds(graph, inf),
    backwardBounds(graph, inf)
{

}

void BidirectionalBoundingRouter::doReset()
{
  forwardBounds.reset(inf);
  backwardBounds.reset(inf);
}


template <bool bounded>
SearchResult
BidirectionalBoundingRouter::findShortestPath(Vertex source,
                                              Vertex target,
                                              const num thetaValue,
                                              const num boundValue)
{
  reset(source, target, thetaValue);

  ReducedCosts reducedCosts(costs, deviations, thetaValue);

  int settled = 0, labeled = 0;
  bool found = false;

  Vertex split;
  num splitValue = inf;

  if(source == target)
  {
    return SearchResult(0, 0, true, Path(), 0);
  }

  LabelHeap<Label> forwardHeap(graph);
  LabelHeap<Label> backwardHeap(graph);

  forwardHeap.update(Label(source, Edge(), 0));
  backwardHeap.update(Label(target, Edge(), 0));

  while(!(forwardHeap.isEmpty() or backwardHeap.isEmpty()))
  {
    const num bestValue = forwardHeap.peek().getCost()
      + backwardHeap.peek().getCost();

    if(bestValue >= splitValue)
    {
      break;
    }

    if(bounded and (bestValue > boundValue))
    {
      break;
    }

    if(forwardHeap.peek().getCost() < backwardHeap.peek().getCost())
    {
      Label current = forwardHeap.extractMin();

      ++settled;

      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        Vertex nextVertex = edge.getTarget();
        num nextCost = current.getCost() + reducedCosts(edge);
        ++labeled;

        num costBound = deviationSize * thetaValue + nextCost;

        if(costBound > forwardBounds(nextVertex))
        {
          continue;
        }
        else
        {
          forwardBounds(nextVertex) = costBound;
        }

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

      ++settled;

      for(const Edge& edge : graph.getIncoming(current.getVertex()))
      {
        Vertex nextVertex = edge.getSource();
        num nextCost = current.getCost() + reducedCosts(edge);
        ++labeled;

        num costBound = deviationSize * thetaValue + nextCost;

        if(costBound > backwardBounds(nextVertex))
        {
          continue;
        }
        else
        {
          backwardBounds(nextVertex) = costBound;
        }

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

SearchResult
BidirectionalBoundingRouter::shortestPath(Vertex source,
                                          Vertex target,
                                          num thetaValue,
                                          num boundValue)
{
  return findShortestPath<true>(source,
                                target,
                                thetaValue,
                                boundValue);
}

SearchResult
BidirectionalBoundingRouter::shortestPath(Vertex source,
                                          Vertex target,
                                          num thetaValue)
{
  return findShortestPath<false>(source,
                                 target,
                                 thetaValue,
                                 inf);
}

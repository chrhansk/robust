#ifndef BIDIRECTIONAL_ACTIVE_ROUTER_HH
#define BIDIRECTIONAL_ACTIVE_ROUTER_HH

#include "active_router.hh"

#include "robust/reduced_costs.hh"

class BidirectionalActiveRouter : public ActiveRouter
{
public:
  BidirectionalActiveRouter(const Graph& graph,
                            const EdgeFunc<num>& costs,
                            const EdgeFunc<num>& deviations,
                            idx deviationSize)
    : ActiveRouter(graph, costs, deviations, deviationSize)
  {}

  template<class ForwardFiler = AllEdgeFilter,
           class BackwardFilter = AllEdgeFilter>
  ActiveSearchResult findShortestPath(Vertex source,
                                      Vertex target,
                                      num value,
                                      ForwardFiler forwardFilter = ForwardFiler(),
                                      BackwardFilter backwardFilter = BackwardFilter());

  ActiveSearchResult findShortestPath(Vertex source, Vertex target, num value) override;
};

template<class ForwardFiler, class BackwardFilter>
ActiveRouter::ActiveSearchResult
BidirectionalActiveRouter::findShortestPath(Vertex source,
                                            Vertex target,
                                            num value,
                                            ForwardFiler forwardFilter,
                                            BackwardFilter backwardFilter)
{
  int settled = 0, labeled = 0;
  bool found = false;

  Vertex split;
  num splitValue = inf;
  ReducedCosts reducedCosts(costs, deviations, value);

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

    if(forwardHeap.peek().getCost() < backwardHeap.peek().getCost())
    {
      Label current = forwardHeap.extractMin();

      ++settled;

      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        if(!forwardFilter(edge))
        {
          continue;
        }

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

      ++settled;

      for(const Edge& edge : graph.getIncoming(current.getVertex()))
      {
        if(!backwardFilter(edge))
        {
          continue;
        }

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

  if(found)
  {
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

    // TODO: Why is this false
    ActiveSearchResult result(SearchResult(settled, labeled, false, path, splitValue));

    for(const Edge& edge : graph.getEdges())
    {
      const Vertex& source = edge.getSource();
      const Vertex& target = edge.getTarget();

      if(forwardHeap.getLabel(source).getState() == State::SETTLED or
         backwardHeap.getLabel(source).getState() == State::SETTLED or
         forwardHeap.getLabel(target).getState() == State::SETTLED or
         backwardHeap.getLabel(target).getState() == State::SETTLED)
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



#endif /* BIDIRECTIONAL_ACTIVE_ROUTER_HH */

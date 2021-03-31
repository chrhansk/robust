#ifndef ARCFLAG_THETA_ROUTER_H
#define ARCFLAG_THETA_ROUTER_H

#include "graph/graph.hh"
#include "router/bidirectional_router.hh"
#include "arcflags/arcflags.hh"
#include "arcflags/arcflag_router.hh"

#include "robust/theta/theta_router.hh"

#include "robust/reduced_costs.hh"

template<class Flags>
class ArcFlagThetaRouter : public ThetaRouter
{
private:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const Partition& partition;
  const Flags& incomingFlags;
  const Flags& outgoingFlags;

  template <bool bounded>
  SearchResult findShortestPath(Vertex source,
                                Vertex target,
                                num value,
                                typename Flags::ThetaFilter forwardFilter,
                                typename Flags::ThetaFilter backwardFilter,
                                num boundValue)
  {
    ReducedCosts reducedCosts(costs, deviations, value);
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
      const num bestValue = std::min(forwardHeap.peek().getCost(),
                                     backwardHeap.peek().getCost());

      if(bestValue >= splitValue)
      {
        break;
      }

      if(bounded and bestValue > boundValue)
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
        assert(forwardFilter(edge));
        path.prepend(edge);
        current = forwardHeap.getLabel(edge.getSource());
      }

      current = backwardHeap.getLabel(split);

      while(!(current.getVertex() == target))
      {
        Edge edge = current.getEdge();
        assert(backwardFilter(edge));
        path.append(edge);
        current = backwardHeap.getLabel(edge.getTarget());
      }

      assert(path.connects(source, target));
      assert(path.cost(reducedCosts) == splitValue);

      return SearchResult(settled, labeled, true, path, splitValue);
    }

    return SearchResult::notFound(settled, labeled);
  }


public:
  ArcFlagThetaRouter(const Graph& graph,
                     const EdgeFunc<num>& costs,
                     const EdgeFunc<num>& deviations,
                     const Partition& partition,
                     const Bidirected<Flags>& flags)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      partition(partition),
      incomingFlags(flags.get(Direction::INCOMING)),
      outgoingFlags(flags.get(Direction::OUTGOING))
  {}

  ArcFlagThetaRouter(const Graph& graph,
                     const EdgeFunc<num>& costs,
                     const EdgeFunc<num>& deviations,
                     const Partition& partition,
                     const Flags& incomingFlags,
                     const Flags& outgoingFlags)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      partition(partition),
      incomingFlags(incomingFlags),
      outgoingFlags(outgoingFlags)
  {}


  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta,
                                    num bound) override
  {
    BidirectionalRouter router(graph);

    const Region& sourceRegion = partition.getRegion(source);
    const Region& targetRegion = partition.getRegion(target);

    if(sourceRegion == targetRegion)
    {
      ReducedCosts reducedCosts(costs, deviations, theta);
      return router.shortestPath<AllEdgeFilter,
                                 AllEdgeFilter,
                                 true>(source,
                                       target,
                                       reducedCosts,
                                       AllEdgeFilter(),
                                       AllEdgeFilter(),
                                       bound);
    }

    typename Flags::ThetaFilter forwardFilter =
      incomingFlags.getThetaFilter(targetRegion, theta);

    typename Flags::ThetaFilter backwardFilter =
      outgoingFlags.getThetaFilter(sourceRegion, theta);

    return findShortestPath<true>(source,
                                  target,
                                  theta,
                                  forwardFilter,
                                  backwardFilter,
                                  bound);
  }

  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta) override
  {
    ReducedCosts reducedCosts(costs, deviations, theta);

    const Region& sourceRegion = partition.getRegion(source);
    const Region& targetRegion = partition.getRegion(target);

    if(sourceRegion == targetRegion)
    {
      BidirectionalRouter router(graph);

      return router.shortestPath<AllEdgeFilter,
                                 AllEdgeFilter,
                                 false>(source,
                                        target,
                                        reducedCosts,
                                        AllEdgeFilter(),
                                        AllEdgeFilter(),
                                        inf);
    }

    typename Flags::ThetaFilter forwardFilter =
      incomingFlags.getThetaFilter(targetRegion, theta);

    typename Flags::ThetaFilter backwardFilter =
      outgoingFlags.getThetaFilter(sourceRegion, theta);

    return findShortestPath<false>(source,
                                   target,
                                   theta,
                                   forwardFilter,
                                   backwardFilter,
                                   inf);
  }
};

#endif /* ARCFLAG_THETA_ROUTER_H */

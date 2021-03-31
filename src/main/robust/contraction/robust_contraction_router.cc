#include "robust_contraction_router.hh"

#include "robust_contraction_preprocessor.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

template <bool upward>
class RobustRankPredicate
{
private:
  const VertexFunc<num>& rankMap;
  const EdgeMap<ContractionRange>& contractionRanges;
  num thetaValue;
public:
  RobustRankPredicate(const VertexFunc<num>& rankMap,
                      const EdgeMap<ContractionRange>& contractionRanges,
                      num thetaValue)
    : rankMap(rankMap),
      contractionRanges(contractionRanges),
      thetaValue(thetaValue)
  {}

  bool operator()(const Edge& edge)
  {
    if(!(contractionRanges(edge).contains(thetaValue)))
    {
      return false;
    }

    if(upward)
    {
      return rankMap(edge.getSource()) < rankMap(edge.getTarget());
    }
    else
    {
      return rankMap(edge.getSource()) > rankMap(edge.getTarget());
    }
  }
};

typedef RobustRankPredicate<true> RobustUpwardPredicate;
typedef RobustRankPredicate<false> RobustDownwardPredicate;

SearchResult RobustContractionRouter::shortestPath(Vertex source,
                                                   Vertex target,
                                                   num theta,
                                                   num bound)
{
  return findShortestPath<true, false>(source, target, theta, bound);
}

SearchResult RobustContractionRouter::shortestPath(Vertex source,
                                                   Vertex target,
                                                   num theta)
{
  return findShortestPath<false, false>(source, target, theta);
}

template<bool bounded,
         bool stalling>
SearchResult RobustContractionRouter::findShortestPath(Vertex source,
                                                       Vertex target,
                                                       num thetaValue,
                                                       num bound)
{
  int settled = 0, labeled = 0;
  bool found = false;

  Vertex split;
  num splitValue = inf;

  const Graph& graph = preprocessor.getOverlayGraph();
  const VertexValueMap<num> ranks = preprocessor.getRankMap().getValues();
  auto costs = preprocessor.getReducedCosts(thetaValue);

  RobustUpwardPredicate upwardPredicate(ranks,
                                        preprocessor.getContractionRanges(),
                                        thetaValue);

  RobustDownwardPredicate downwardPredicate(ranks,
                                            preprocessor.getContractionRanges(),
                                            thetaValue);

  LabelHeap<Label> forwardHeap(graph);
  LabelHeap<Label> backwardHeap(graph);

  forwardHeap.update(Label(source, Edge(), 0));
  backwardHeap.update(Label(target, Edge(), 0));

  while(!(forwardHeap.isEmpty() and backwardHeap.isEmpty()))
  {
    num backwardValue = backwardHeap.isEmpty() ? inf : backwardHeap.peek().getCost();
    num forwardValue = forwardHeap.isEmpty() ? inf : forwardHeap.peek().getCost();

    const num boundValue = bounded ?
      std::min(splitValue, bound) :
      splitValue;

    if(bounded)
    {
      if(forwardValue > boundValue and backwardValue > splitValue)
      {
        break;
      }
    }

    if(forwardValue < backwardValue)
    {
      Label current = forwardHeap.extractMin();

      ++settled;

      if(stalling)
      {
        bool found = false;
        for(const Edge& edge : graph.getIncoming(current.getVertex()))
        {
          if(upwardPredicate(edge))
          {
            continue;
          }

          const Label& previous = forwardHeap.getLabel(edge.getSource());

          if(previous.getState() != State::UNKNOWN)
          {
            if(previous.getCost() + costs(edge) < current.getCost())
            {
              found = true;
              break;
            }
          }
        }

        if(found)
        {
          continue;
        }
      }

      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        if(!upwardPredicate(edge))
        {
          continue;
        }

        Vertex nextVertex = edge.getTarget();
        num nextCost = current.getCost() + costs(edge);
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

      if(stalling)
      {
        bool found = false;
        for(const Edge& edge : graph.getOutgoing(current.getVertex()))
        {
          if(downwardPredicate(edge))
          {
            continue;
          }

          const Label& previous = backwardHeap.getLabel(edge.getTarget());

          if(previous.getState() != State::UNKNOWN)
          {
            if(previous.getCost() + costs(edge) < current.getCost())
            {
              found = true;
              break;
            }
          }
        }

        if(found)
        {
          continue;
        }
      }

      for(const Edge& edge : graph.getIncoming(current.getVertex()))
      {
        if(!downwardPredicate(edge))
        {
          continue;
        }

        Vertex nextVertex = edge.getSource();
        num nextCost = current.getCost() + costs(edge);

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
      if(splitValue > bound)
      {
        return SearchResult(settled, labeled, false, Path());
      }
    }

    Path path;

    Label current = forwardHeap.getLabel(split);

    while(!(current.getVertex() == source))
    {
      Edge edge = current.getEdge();
      assert(upwardPredicate(edge));
      path.prepend(edge);
      current = forwardHeap.getLabel(edge.getSource());
    }

    current = backwardHeap.getLabel(split);

    while(!(current.getVertex() == target))
    {
      Edge edge = current.getEdge();
      assert(downwardPredicate(edge));
      path.append(edge);
      current = backwardHeap.getLabel(edge.getTarget());
    }

    assert(path.connects(source, target));
    assert(path.cost(costs) == splitValue);

    if(bounded)
    {
      assert(splitValue <= bound);
    }

    //assert(validContractedPath(path));

    path = preprocessor.unpack(path);

    return SearchResult(settled, labeled, true, path);
  }

  return SearchResult(settled, labeled, false, Path());
}

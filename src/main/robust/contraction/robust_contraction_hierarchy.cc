#include "robust_contraction_hierarchy.hh"

#include <stack>

#include "router/label.hh"
#include "router/label_heap.hh"

class RobustContractionLabel : public AbstractLabel
{
private:
  const RobustContractionEdge* edge;

public:
  RobustContractionLabel()
    : edge(nullptr)
  {}
  RobustContractionLabel(Vertex vertex,
                   const RobustContractionEdge& edge,
                   num cost)
    : AbstractLabel(vertex, cost),
      edge(&edge)
  {}

  const RobustContractionEdge& getEdge() const
  {
    return *edge;
  }

};


RobustContractionHierarchy::RobustContractionHierarchy(const Graph& overlayGraph,
                                                       const EdgeFunc<const ContractionRange&>& contractionRanges,
                                                       const VertexMap<num>& ranks,
                                                       const EdgeFunc<const EdgePair&>& edgePairs)
  : graph(Graph(overlayGraph.getVertices().size(), {})),
    permutation(graph, Vertex()),
    upwardEdges(graph, std::vector<RobustContractionEdge>()),
    downwardEdges(graph, std::vector<RobustContractionEdge>()),
    originalEdges(overlayGraph, EdgePair())
{
  for(const Vertex& originalVertex : overlayGraph.getVertices())
  {
    const Vertex& permutedVertex = overlayGraph.getVertices()[ranks(originalVertex)];
    permutation(originalVertex) = permutedVertex;
  }

  for(const Edge& edge : overlayGraph.getEdges())
  {
    const Vertex& source = edge.getSource();
    const Vertex& target = edge.getTarget();

    bool upwards = ranks(source) < ranks(target);

    Vertex permutedSource = permutation(source);
    Vertex permutedTarget = permutation(target);

    originalEdges(edge) = edgePairs(edge);

    if(upwards)
    {
      RobustContractionEdge contractionEdge(permutedTarget,
                                            edge,
                                            contractionRanges(edge));

      upwardEdges(permutedSource).push_back(contractionEdge);
    }
    else
    {
      RobustContractionEdge contractionEdge(permutedSource,
                                            edge,
                                            contractionRanges(edge));

      downwardEdges(permutedTarget).push_back(contractionEdge);
    }
  }
}

SearchResult RobustContractionHierarchy::Router::shortestPath(Vertex source,
                                                              Vertex target,
                                                              num theta)
{
  return findShortestPath<false>(source, target, theta);
}

SearchResult RobustContractionHierarchy::Router::shortestPath(Vertex source,
                                                        Vertex target,
                                                        num theta,
                                                        num bound)
{
  return findShortestPath<true>(source, target, theta, bound);
}


template <bool bounded>
SearchResult RobustContractionHierarchy::Router::findShortestPath(Vertex source,
                                                                  Vertex target,
                                                                  num theta,
                                                                  num bound)
{
  Vertex permutedSource = hierarchy.permutation(source);
  Vertex permutedTarget = hierarchy.permutation(target);

  LabelHeap<RobustContractionLabel> forwardHeap(hierarchy.graph);
  LabelHeap<RobustContractionLabel> backwardHeap(hierarchy.graph);

  forwardHeap.update(RobustContractionLabel(permutedSource, RobustContractionEdge(), 0));
  backwardHeap.update(RobustContractionLabel(permutedTarget, RobustContractionEdge(), 0));

  int settled = 0, labeled = 0;
  bool found = false;

  Vertex split;
  num splitValue = inf;

  while(!(forwardHeap.isEmpty() and backwardHeap.isEmpty()))
  {
    num backwardValue = backwardHeap.isEmpty() ? inf : backwardHeap.peek().getCost();
    num forwardValue = forwardHeap.isEmpty() ? inf : forwardHeap.peek().getCost();

    const num boundValue = bounded ?
      std::min(splitValue, bound) :
      splitValue;

    if(forwardValue >= boundValue and
       backwardValue >= boundValue)
    {
      break;
    }

    if(forwardValue < backwardValue)
    {
      RobustContractionLabel current = forwardHeap.extractMin();

      ++settled;

      for(const RobustContractionEdge& edge : hierarchy.upwardEdges(current.getVertex()))
      {
        if(!edge.getRange().contains(theta))
        {
          continue;
        }

        Vertex nextVertex = edge.getVertex();
        num nextCost = current.getCost() + edge.getRange().getReducedCost(theta);
        ++labeled;

        forwardHeap.update(RobustContractionLabel(nextVertex, edge, nextCost));

        const RobustContractionLabel& other = backwardHeap.getLabel(nextVertex);

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
      RobustContractionLabel current = backwardHeap.extractMin();

      ++settled;

      for(const RobustContractionEdge& edge : hierarchy.downwardEdges(current.getVertex()))
      {
        if(!edge.getRange().contains(theta))
        {
          continue;
        }

        Vertex nextVertex = edge.getVertex();
        num nextCost = current.getCost() + edge.getRange().getReducedCost(theta);
        ++labeled;

        backwardHeap.update(RobustContractionLabel(nextVertex, edge, nextCost));

        const RobustContractionLabel& other = forwardHeap.getLabel(nextVertex);

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
        return SearchResult::notFound(settled, labeled);
      }
    }

    Path path;

    RobustContractionLabel current = forwardHeap.getLabel(split);

    while(current.getVertex() != permutedSource)
    {
      Edge edge = current.getEdge().getEdge();
      path.prepend(edge);
      current = forwardHeap.getLabel(hierarchy.permutation(edge.getSource()));
    }

    current = backwardHeap.getLabel(split);

    while(current.getVertex() != permutedTarget)
    {
      Edge edge = current.getEdge().getEdge();
      path.append(edge);
      current = backwardHeap.getLabel(hierarchy.permutation(edge.getTarget()));
    }

    assert(path.connects(source, target));

    if(bounded)
    {
      assert(splitValue <= bound);
    }

    path = hierarchy.unpack(path);

    return SearchResult(settled, labeled, true, path, splitValue);
  }

  return SearchResult::notFound(settled, labeled);
}

Path RobustContractionHierarchy::unpack(const Path& overlayPath) const
{
  Path path;

  std::stack<Edge> edges;

  for(const Edge& edge : overlayPath.getEdges())
  {
    edges.push(edge);
  }

  while(!edges.empty())
  {
    const Edge& edge = edges.top();
    edges.pop();
    const EdgePair& pair = originalEdges(edge);

    if(pair.first == edge)
    {
      path.prepend(edge);
    }
    else
    {
      edges.push(pair.first);
      edges.push(pair.second);
    }
  }

  return path;
}

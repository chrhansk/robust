#include "contraction_hierarchy.hh"

#include <stack>

#include "router/label.hh"
#include "router/label_heap.hh"

class ContractionLabel : public AbstractLabel
{
private:
  const ContractionEdge* edge;

public:
  ContractionLabel()
    : edge(nullptr)
  {}
  ContractionLabel(Vertex vertex,
                   const ContractionEdge& edge,
                   num cost)
    : AbstractLabel(vertex, cost),
      edge(&edge)
  {}

  const ContractionEdge& getEdge() const
  {
    return *edge;
  }

};


ContractionHierarchy::ContractionHierarchy(const Graph& overlayGraph,
                                           const EdgeFunc<num>& overlayCosts,
                                           const VertexMap<num>& ranks,
                                           const EdgeFunc<EdgePair>& originalEdges)
  : graph(Graph(overlayGraph.getVertices().size(), {})),
    permutation(graph, Vertex()),
    upwardEdges(graph, std::vector<ContractionEdge>()),
    downwardEdges(graph, std::vector<ContractionEdge>()),
    originalEdges(overlayGraph, originalEdges)
{
  for(const Vertex& originalVertex : graph.getVertices())
  {
    const Vertex& permutedVertex = graph.getVertices()[ranks(originalVertex)];
    permutation(originalVertex) = permutedVertex;
  }

  for(const Edge& edge : overlayGraph.getEdges())
  {
    const Vertex& source = edge.getSource();
    const Vertex& target = edge.getTarget();

    bool upwards = ranks(source) < ranks(target);

    Vertex permutedSource = permutation(source);
    Vertex permutedTarget = permutation(target);

    if(upwards)
    {
      ContractionEdge contractionEdge(permutedTarget,
                                      edge,
                                      overlayCosts(edge));

      upwardEdges(permutedSource).push_back(contractionEdge);
    }
    else
    {
      ContractionEdge contractionEdge(permutedSource,
                                      edge,
                                      overlayCosts(edge));

      downwardEdges(permutedTarget).push_back(contractionEdge);
    }
  }
}


SearchResult ContractionHierarchy::Router::shortestPath(Vertex source,
                                                        Vertex target,
                                                        const EdgeFunc<num>& costs)
{
  return findShortestPath<false>(source, target);
}

SearchResult ContractionHierarchy::Router::shortestPath(Vertex source,
                                                        Vertex target,
                                                        const EdgeFunc<num>& costs,
                                                        num bound)
{
  return findShortestPath<true>(source, target, bound);
}

template <bool bounded>
SearchResult ContractionHierarchy::Router::findShortestPath(Vertex source,
                                                            Vertex target,
                                                            num bound)
{
  Vertex permutedSource = hierarchy.permutation(source);
  Vertex permutedTarget = hierarchy.permutation(target);

  LabelHeap<ContractionLabel> forwardHeap(hierarchy.graph);
  LabelHeap<ContractionLabel> backwardHeap(hierarchy.graph);

  forwardHeap.update(ContractionLabel(permutedSource, ContractionEdge(), 0));
  backwardHeap.update(ContractionLabel(permutedTarget, ContractionEdge(), 0));

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
      ContractionLabel current = forwardHeap.extractMin();

      ++settled;

      for(const ContractionEdge& edge : hierarchy.upwardEdges(current.getVertex()))
      {
        Vertex nextVertex = edge.vertex;
        num nextCost = current.getCost() + edge.cost;
        ++labeled;

        forwardHeap.update(ContractionLabel(nextVertex, edge, nextCost));

        const ContractionLabel& other = backwardHeap.getLabel(nextVertex);

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
      ContractionLabel current = backwardHeap.extractMin();

      ++settled;

      for(const ContractionEdge& edge : hierarchy.downwardEdges(current.getVertex()))
      {
        Vertex nextVertex = edge.vertex;
        num nextCost = current.getCost() + edge.cost;
        ++labeled;

        backwardHeap.update(ContractionLabel(nextVertex, edge, nextCost));

        const ContractionLabel& other = forwardHeap.getLabel(nextVertex);

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

    ContractionLabel current = forwardHeap.getLabel(split);

    while(current.getVertex() != permutedSource)
    {
      Edge edge = current.getEdge().edge;
      path.prepend(edge);
      current = forwardHeap.getLabel(hierarchy.permutation(edge.getSource()));
    }

    current = backwardHeap.getLabel(split);

    while(current.getVertex() != permutedTarget)
    {
      Edge edge = current.getEdge().edge;
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

Path ContractionHierarchy::unpack(const Path& overlayPath) const
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

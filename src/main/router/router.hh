#ifndef ROUTER_HH
#define ROUTER_HH

#include "graph/edge_map.hh"
#include "graph/graph.hh"
#include "path/path.hh"

#include "label.hh"
#include "label_heap.hh"

class AllEdgeFilter
{
public:
  constexpr bool operator()(const Edge& edge) const
  {
    return true;
  }
};

class NoEdgeFilter
{
public:
  constexpr bool operator()(const Edge& edge) const
  {
    return false;
  }
};

/**
 * A SearchResult represents the result of a shortest Path
 * search. The search can be successful (a Path was found)
 * or unsuccessful (no Path was found). If the search
 * was successful, the SearchResult contains the corresponding
 * path.
 **/
class SearchResult
{

public:
  SearchResult(int settled, int labeled, bool found, Path path, num cost)
    : settled(settled),
      labeled(labeled),
      found(found),
      path(path),
      cost(cost)
  {}

  SearchResult()
    : settled(0),
      labeled(0),
      found(false),
      cost(0)
  {}
  SearchResult(const SearchResult& other) = default;

  static SearchResult notFound(idx settled, idx labeled)
  {
    return SearchResult(settled, labeled, false, Path(), 0);
  }

  int settled;
  int labeled;
  bool found;
  Path path;
  num cost;
};

/**
 * A base class for all routers.
 **/
class Router
{
public:
  /**
   * Finds the shortest path between the given vertices
   * with respect to the given (non-negative) costs.
   *
   * @param source The source Vertex
   * @param target The target Vertex
   * @param costs  A function containing (non-negative) costs.
   *
   * @return A SearchResult
   **/
  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    const EdgeFunc<num>& costs) = 0;

  /**
   * Finds the shortest path between the given vertices
   * with respect to the given (non-negative) costs.
   *
   * @param source The source Vertex
   * @param target The target Vertex
   * @param costs  A function containing (non-negative) costs.
   * @param bound  An upper bound on the path cost. If a path
   *               is returned it is guaranteed to have a cost
   *               of at most the given bound value. If it is
   *               determined that no path satisfiying the given
   *               bound exists, then the computation can be
   *               aborted early.
   *
   * @return A SearchResult
   **/
  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    const EdgeFunc<num>& costs,
                                    num bound) = 0;

  virtual ~Router() {}
};

/**
 * A class which find shortest paths by performing a unidirectional
 * search from the source Vertex.
 **/
class Dijkstra : public Router
{
private:
  const Graph& graph;

public:
  Dijkstra(const Graph& graph)
    : graph(graph) {}

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs) override;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs,
                            num bound) override;

  /**
   * A function returning a shortest path which is bounded
   * and which in addition satisfies the given filter.
   *
   * @tparam Filter  A filter given by a function mapping from Edge%s
   *                 to boolean values
   * @tparam bounded Whether or not to respect the given bound value.
   **/
  template<class Filter = AllEdgeFilter, bool bounded = false>
  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs,
                            const Filter& filter = Filter(),
                            num bound = inf);
};


template<class Filter, bool bounded>
SearchResult Dijkstra::shortestPath(Vertex source,
                                    Vertex target,
                                    const EdgeFunc<num>& costs,
                                    const Filter& filter,
                                    num bound)
{
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
      if(current.getCost() > bound)
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
      if(!filter(edge))
      {
        continue;
      }

      ++labeled;

      Label nextLabel = Label(edge.getTarget(),
                              edge, current.getCost() + costs(edge));

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

    return SearchResult(settled, labeled, true, path, cost);
  }

  return SearchResult::notFound(settled, labeled);
}

#endif /* ROUTER_HH */

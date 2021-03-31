#ifndef DISTANCE_TREE_HH
#define DISTANCE_TREE_HH

#include <stdexcept>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "label.hh"
#include "label_heap.hh"
#include "router.hh"

template <Direction direction>
class DistanceTree
{
private:
  const Graph& graph;
  const EdgeFunc<num>& costs;

  LabelHeap<SimpleLabel> heap;
  num maxDist;

  template<class Predicate, class Filter = AllEdgeFilter>
  void extendWhile(Predicate predicate, Filter filter = Filter());

public:
  DistanceTree(const Graph& graph,
               const EdgeFunc<num>& costs);

  DistanceTree(const Graph& graph,
               const EdgeFunc<num>& costs,
               Vertex root);

  template <class It>
  DistanceTree(const Graph& graph,
               const EdgeFunc<num>& costs,
               It begin,
               It end);

  template<class Filter = AllEdgeFilter>
  void extend(Filter filter = Filter());

  template<class Filter = AllEdgeFilter>
  Vertex next(Filter filter = Filter());

  template<class Filter = AllEdgeFilter>
  void extend(Vertex vertex, Filter filter = Filter());

  template <class It, class Filter = AllEdgeFilter>
  void extend(It begin, It end, Filter filter = Filter());

  bool explored(Vertex vertex) const;

  num distance(Vertex vertex) const;

  void add(Vertex vertex, num distance = 0);

  bool done() const
  {
    return heap.isEmpty();
  }

  num maxDistance() const
  {
    return maxDist;
  }

};

template <Direction direction>
DistanceTree<direction>::DistanceTree(const Graph& graph,
                                      const EdgeFunc<num>& costs)
  : graph(graph),
    costs(costs),
    heap(graph),
    maxDist(0)
{
}

template <Direction direction>
DistanceTree<direction>::DistanceTree(const Graph& graph,
                                      const EdgeFunc<num>& costs,
                                      Vertex root)
  : graph(graph),
    costs(costs),
    heap(graph),
    maxDist(0)
{
  add(root);
}

template <Direction direction>
template <class It>
DistanceTree<direction>::DistanceTree(const Graph& graph,
                                      const EdgeFunc<num>& costs,
                                      It begin,
                                      It end)
  : graph(graph),
    costs(costs),
    heap(graph)
{
  for(auto it = begin; it != end; ++it)
  {
    add(*it);
  }
}

template <Direction direction>
template <class Predicate, class Filter>
void DistanceTree<direction>::extendWhile(Predicate predicate, Filter filter)
{
  while(!heap.isEmpty() and predicate())
  {
    next(filter);
  }
}

template <Direction direction>
template <class Filter>
Vertex DistanceTree<direction>::next(Filter filter)
{
  assert(!done());

  const SimpleLabel& current = heap.extractMin();

  maxDist = std::max(maxDist, current.getCost());

  for(const Edge& edge : graph.getEdges(current.getVertex(), direction))
  {
    if(!filter(edge))
    {
      continue;
    }

    SimpleLabel nextLabel = SimpleLabel(edge.getEndpoint(direction),
                                        current.getCost() + costs(edge));

    heap.update(nextLabel);
  }

  return current.getVertex();
}

template <Direction direction>
template <class Filter>
void DistanceTree<direction>::extend(Filter filter)
{
  extendWhile([] () -> bool {return true;}, filter);
}

template<Direction direction>
template<class Filter>
void DistanceTree<direction>::extend(Vertex vertex, Filter filter)
{
  extendWhile([&] () -> bool {return !explored(vertex);}, filter);
}

template <Direction direction>
template <class It, class Filter>
void DistanceTree<direction>::extend(It begin, It end, Filter filter)
{
  for(auto it = begin; it != end; ++it)
  {
    extendWhile([&] () -> bool {return !explored(*it);}, filter);
  }
}

template <Direction direction>
bool DistanceTree<direction>::explored(Vertex vertex) const
{
  return heap.getLabel(vertex).getState() == State::SETTLED;
}

template <Direction direction>
num DistanceTree<direction>::distance(Vertex vertex) const
{
  if(!explored(vertex))
  {
    throw std::invalid_argument("Vertex has not been explored");
  }

  return heap.getLabel(vertex).getCost();
}

template <Direction direction>
void DistanceTree<direction>::add(Vertex vertex, num distance)
{
  if(explored(vertex))
  {
    throw std::invalid_argument("Vertex has already been explored");
  }

  maxDist = std::max(maxDist, distance);

  return heap.update(SimpleLabel(vertex, distance));
}

#endif /* DISTANCE_TREE_HH */

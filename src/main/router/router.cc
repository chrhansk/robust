#include "router.hh"

#include "graph/vertex_map.hh"

#include "label.hh"
#include "label_heap.hh"

SearchResult Dijkstra::shortestPath(Vertex source,
                                    Vertex target,
                                    const EdgeFunc<num>& costs)
{
  return shortestPath<AllEdgeFilter, false>(source,
                                            target,
                                            costs,
                                            AllEdgeFilter(),
                                            inf);
}

SearchResult Dijkstra::shortestPath(Vertex source,
                                    Vertex target,
                                    const EdgeFunc<num>& costs,
                                    num bound)
{
  if(bound == inf)
  {
    return shortestPath<AllEdgeFilter, false>(source,
                                              target,
                                              costs,
                                              AllEdgeFilter(),
                                              bound);
  }
  else
  {
    return shortestPath<AllEdgeFilter, true>(source,
                                             target,
                                             costs,
                                             AllEdgeFilter(),
                                             bound);
  }
}

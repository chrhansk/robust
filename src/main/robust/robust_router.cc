#include "robust_router.hh"

#include <algorithm>
#include <functional>

void RobustSearchResult::add(const SearchResult& other)
{
  ++calls;

  numFound += idx(other.found);
  settled += other.settled;
  labeled += other.labeled;
}

RobustSearchResult RobustRouter::shortestPath(Vertex source, Vertex target)
{
  return shortestPath(source, target, inf);
}

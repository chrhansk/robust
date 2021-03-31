#ifndef ACTIVE_ROUTER_HH
#define ACTIVE_ROUTER_HH

#include "robust/robust_router.hh"
#include "robust/robust_utils.hh"
#include "robust/value_range.hh"

class ActiveRouter : public RobustRouter
{
protected:

  class ActiveSearchResult : public SearchResult
  {
  private:
    std::vector<Edge> activeEdges;

  public:
    ActiveSearchResult(const SearchResult& searchResult)
      : SearchResult(searchResult)
    {}

    const std::vector<Edge>& getActiveEdges() const
    {
      return activeEdges;
    }

    std::vector<Edge>& getActiveEdges()
    {
      return activeEdges;
    }
  };

  virtual ActiveSearchResult findShortestPath(Vertex source, Vertex target, num value) = 0;

  virtual ActiveSearchResult findMaxShortestPath(Vertex source, Vertex target, num value)
  {
    return findShortestPath(source, target, value);
  }

  /*
    float getLowerBound(const ActiveInterval& interval,
    const num value);
  */

  num getPathCost(const Path& path, num value);

public:
  ActiveRouter(const Graph& graph,
               const EdgeFunc<num>& costs,
               const EdgeFunc<num>& deviations,
               idx deviationSize);

  using RobustRouter::shortestPath;

  RobustSearchResult shortestPath(Vertex source,
                                  Vertex target,
                                  const ValueVector& possibleValues,
                                  num bound) override;
};


#endif /* ACTIVE_ROUTER_HH */

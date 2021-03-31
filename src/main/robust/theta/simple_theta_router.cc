#include "simple_theta_router.hh"

#include "robust/reduced_costs.hh"

SimpleThetaRouter::SimpleThetaRouter(const Graph& graph,
                                     const EdgeFunc<num>& costs,
                                     const EdgeFunc<num>& deviations,
                                     idx deviationSize)
  : BidirectionalRouter(graph), costs(costs), deviations(deviations)
{

}

SearchResult SimpleThetaRouter::shortestPath(Vertex source,
                                             Vertex target,
                                             num theta,
                                             num bound)
{
  return BidirectionalRouter::shortestPath<AllEdgeFilter,
                                           AllEdgeFilter,
                                           true>(source,
                                                 target,
                                                 ReducedCosts(costs, deviations, theta),
                                                 AllEdgeFilter(),
                                                 AllEdgeFilter(),
                                                 bound);
}

SearchResult SimpleThetaRouter::shortestPath(Vertex source,
                                             Vertex target,
                                             num theta)
{
  return BidirectionalRouter::shortestPath<AllEdgeFilter,
                                           AllEdgeFilter,
                                           false>(source,
                                                  target,
                                                  ReducedCosts(costs, deviations, theta),
                                                  AllEdgeFilter(),
                                                  AllEdgeFilter());
}

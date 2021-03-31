#include "theta_router.hh"

SearchResult ThetaRouter::shortestPath(Vertex source,
                                       Vertex target,
                                       num theta)
{
  return shortestPath(source, target, theta, inf);
}

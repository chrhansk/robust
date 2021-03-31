#ifndef THETA_ROUTER_HH
#define THETA_ROUTER_HH

#include "util.hh"

#include "router/router.hh"

/**
 * A ThetaRouter computes shortest Path%s between
 * the vertices of a given Graph with respect to
 * the ReducedCosts defined by costs, deviations
 * and a value for \f$ \theta \f$.
 **/
class ThetaRouter
{
public:

  /**
   * Computes a shortest Path between the
   * given vertices with respect to the
   * ReducedCosts defined by the given value
   * \f$ \theta \f$. The cost of the
   * Path with respect to the ReducedCosts
   * is required to be at most equal
   * to the given bound value.
   *
   * @param source The source vertex.
   * @param target The target vertex.
   * @param theta  The value \f$ \theta \f$.
   * @param bound  The cost bound.
   *
   * @return A SearchResult
   */
  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta,
                                    num bound) = 0;

  /**
   * Computes a shortest Path between the
   * given vertices with respect to the
   * ReducedCosts defined by the given value
   * \f$ \theta \f$.
   *
   * @param source The source vertex.
   * @param target The target vertex.
   * @param theta  The value \f$ \theta \f$.
   *
   * @return A SearchResult
   */
  virtual SearchResult shortestPath(Vertex source,
                                    Vertex target,
                                    num theta);
};

#endif /* THETA_ROUTER_HH */

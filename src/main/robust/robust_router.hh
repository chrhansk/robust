#ifndef ROBUST_ROUTER_HH
#define ROBUST_ROUTER_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "path/path.hh"

#include "router/router.hh"

#include "robust_utils.hh"

class RobustSearchResult
{
public:
  RobustSearchResult(idx calls,
                     idx numFound,
                     idx settled,
                     idx labeled,
                     bool found,
                     Path path)
    : calls(calls),
      numFound(numFound),
      settled(settled),
      labeled(labeled),
      found(found),
      path(path)
  {}

  RobustSearchResult()
    : calls(0),
      numFound(0),
      settled(0),
      labeled(0),
      found(false)
  {}

  idx calls;
  idx numFound;
  idx settled;
  idx labeled;
  bool found;
  Path path;

  void add(const SearchResult& other);
};


/**
 * A base class for all robust shortest path algorithms.
 * A robust shortest path is a path which is minimal
 * with respect to its robust costs.
 * @see RobustCosts
 **/
class RobustRouter
{
protected:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const idx deviationSize;
  ValueVector values;

public:

  /**
   * Constructs a new RobustRouter
   *
   * @param graph          The underlying Graph.
   * @param costs          The costs \f$ c \f$.
   * @param deviations     The deviations \f$ d \f$.
   * @param deviationSize  the parameter \f$ \Gamma \f$.
   **/
  RobustRouter(const Graph& graph,
               const EdgeFunc<num>& costs,
               const EdgeFunc<num>& deviations,
               const idx deviationSize)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      deviationSize(deviationSize),
      values(thetaValues(graph, deviations)) {}

  /**
   * Computes a robust shortest path.
   *
   * @param source The source vertex.
   * @param target The target vertex.
   **/
  virtual RobustSearchResult shortestPath(Vertex source,
                                          Vertex target);

  /**
   * Computes a robust shortest path with a bounded
   * total cost. @see Router
   *
   * @param source         The source vertex.
   * @param target         The target vertex.
   * @param possibleValues The theta values which can occur.
   * @param bound          The cost bound.
   **/
  virtual RobustSearchResult shortestPath(Vertex source,
                                          Vertex target,
                                          const ValueVector& possibleValues,
                                          num bound) = 0;

  virtual RobustSearchResult shortestPath(Vertex source,
                                          Vertex target,
                                          num bound)
  {
    return shortestPath(source, target, values, bound);
  }
};

#endif /* ROBUST_ROUTER_HH */

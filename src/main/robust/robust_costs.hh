#ifndef ROBUST_COSTS_HH
#define ROBUST_COSTS_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "path/path.hh"
#include "robust_utils.hh"

/**
 * The robust cost of a path \f$P\f$ with respect to costs
 * \f$ c : A \mapsto \mathbb{R}_{\geq 0} \f$,
 * deviations
 * \f$ d : A \mapsto \mathbb{R}_{\geq 0} \f$
 * and a paramter \f$ \Gamma \in \mathbb{N} \f$
 * is given by
 *
 * \f[
 *  c(P) + \max_{S \subseteq A(P), |S| \leq \Gamma} \sum_{a \in S} d(a)
 * \f]
 *
 * This class can be used to compute the robust costs of given Path%s.
 *
 **/
class RobustCosts
{
private:
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  idx deviationSize;

public:
  /**
   * Constructs new RobustCosts
   *
   * @param costs          The costs \f$ c \f$.
   * @param deviations     The deviations \f$ d \f$.
   * @param deviationSize  the parameter \f$ \Gamma \f$.
   **/
  RobustCosts(const EdgeFunc<num>& costs,
              const EdgeFunc<num>& deviations,
              idx deviationSize);

  /**
   * Computes the robust costs of a given Path.
   **/
  num get(const Path& path);

  num optimalCost(const Path& path,
                  const ValueVector& values);
};



#endif /* ROBUST_COSTS_HH */

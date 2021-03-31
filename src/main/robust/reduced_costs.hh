#ifndef REDUCED_COSTS_HH
#define REDUCED_COSTS_HH

#include <cassert>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

/**
 * The reduced costs with respect to costs \f$ c : A \rightarrow \mathbb{R}_{\geq 0} \f$,
 * deviations \f$ d : A \rightarrow \mathbb{R}_{\geq 0} \f$,
 * and a value \f$\theta \geq 0 \f$ are defined as
 *
 * \f[
 * a \mapsto c(a) + \max(d(a) - \theta, 0)
 * \f]
 *
 * This class can be used in order to evaluate the reduced costs.
 *
 * @param costs      the costs \f$ c \f$
 * @param deviations the deviations \f$ d \f$
 * @param theta      the value of \f$ \theta \f$
 *
 * @return
 */
class ReducedCosts : public EdgeFunc<num>
{
private:
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  num theta;

public:
  ReducedCosts(const EdgeFunc<num>& costs,
               const EdgeFunc<num>& deviations,
               num theta)
    : costs(costs), deviations(deviations), theta(theta) {}

  num operator()(const Edge& edge) const override
  {
    num cost = costs(edge);
    num deviation = deviations(edge);
    num reduced = cost + std::max(deviation - theta, (num) 0);
    assert(reduced >= 0);
    return reduced;
  }
};

#endif /* REDUCED_COSTS_HH */

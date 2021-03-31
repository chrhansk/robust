#include "abstract_arcflag_preprocessor.hh"

#include "log.hh"

AbstractArcFlagPreprocessor::AbstractArcFlagPreprocessor(const Graph& graph,
                                                         const EdgeFunc<num>& costs,
                                                         const EdgeFunc<num>& deviations,
                                                         const Partition& partition)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    partition(partition),
    values(thetaValues(graph, deviations))
{
}

AbstractArcFlagPreprocessor::AbstractArcFlagPreprocessor(const Graph& graph,
                                                         const EdgeFunc<num>& costs,
                                                         const EdgeFunc<num>& deviations,
                                                         const Partition& partition,
                                                         const ValueVector& values)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    partition(partition),
    values(values)
{
}

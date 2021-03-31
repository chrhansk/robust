#ifndef ABSTRACT_ARCFLAG_PREPROCESSOR_HH
#define ABSTRACT_ARCFLAG_PREPROCESSOR_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"

#include "robust_arcflags.hh"

#include "robust/robust_utils.hh"

class AbstractArcFlagPreprocessor
{
protected:
  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const Partition& partition;
  ValueVector values;

public:
  AbstractArcFlagPreprocessor(const Graph& graph,
                              const EdgeFunc<num>& costs,
                              const EdgeFunc<num>& deviations,
                              const Partition& partition);

  AbstractArcFlagPreprocessor(const Graph& graph,
                              const EdgeFunc<num>& costs,
                              const EdgeFunc<num>& deviations,
                              const Partition& partition,
                              const ValueVector& values);

  virtual ~AbstractArcFlagPreprocessor() {}
};


#endif /* ABSTRACT_ARCFLAG_PREPROCESSOR_HH */

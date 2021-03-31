#ifndef ROBUST_ARCFLAG_PREPROCESSOR_HH
#define ROBUST_ARCFLAG_PREPROCESSOR_HH

#include <unordered_set>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"

#include "abstract_arcflag_preprocessor.hh"
#include "robust_arcflags.hh"

/**
 * The robust arc flag preprocessor computes RobustArcFlags
 * by determining the outgoing / incoming trees of the boundary
 * vertices of the Region%s of a given Partition for all
 * values of \f$ \theta \f$ one after another. The process
 * can be parallelized.
 **/
class RobustArcFlagPreprocessor : public AbstractArcFlagPreprocessor
{
private:

  template <Direction direction>
  void computeFlags(const Vertex& vertex,
                    RobustArcFlags& flags) const;

  template <Direction direction>
  void computeFlagsParallel(const std::unordered_set<Vertex>& vertices,
                            RobustArcFlags& flags) const;

public:
  RobustArcFlagPreprocessor(const Graph& graph,
                            const EdgeFunc<num>& costs,
                            const EdgeFunc<num>& deviations,
                            const Partition& partition);

  RobustArcFlagPreprocessor(const Graph& graph,
                            const EdgeFunc<num>& costs,
                            const EdgeFunc<num>& deviations,
                            const Partition& partition,
                            const ValueVector& values);

  void computeFlags(RobustArcFlags& incomingFlags,
                    RobustArcFlags& outgoingFlags,
                    bool parallelComputation) const;

  template<class Flags>
  void computeFlags(Bidirected<Flags>& flags,
                    bool parallelComputation) const;
};

template<class Flags>
void RobustArcFlagPreprocessor::computeFlags(Bidirected<Flags>& flags,
                                             bool parallelComputation) const
{
  computeFlags(flags.get(Direction::INCOMING),
               flags.get(Direction::OUTGOING),
               parallelComputation);
}


#endif /* ROBUST_ARCFLAG_PREPROCESSOR_HH */

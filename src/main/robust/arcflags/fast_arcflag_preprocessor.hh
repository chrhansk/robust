#ifndef FAST_ARCFLAG_PREPROCESSOR_HH
#define FAST_ARCFLAG_PREPROCESSOR_HH

#include <unordered_set>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "arcflags/arcflags.hh"
#include "arcflags/partition.hh"

#include "robust/robust_utils.hh"

#include "abstract_arcflag_preprocessor.hh"
#include "robust_arcflags.hh"

/**
 * The fast arcflag preprpocessor computes RobustArcFlags by
 * determining outgoing / incoming trees for the boundary
 * vertices of the Region%s of a given Partition. Only
 * a limited number of trees is actually computed. For each
 * additional Edge, flags are extended iff the Edge may be needed.
 * This approach is much faster than the one employed by the
 * RobustArcFlagPreprocessor, however, it can result in flags
 * being extended more than necessary.
 **/
class FastArcFlagPreprocessor : public AbstractArcFlagPreprocessor
{
private:

  template <Direction direction,
            class OutIt>
  void computeTree(Vertex root,
                   const Region& region,
                   const EdgeFunc<num>& reducedCosts,
                   LabelHeap<Label>& heap,
                   OutIt outIt) const;

  template<Direction direction,
           class OutIt>
  void collectAdditionalEdges(Vertex root,
                              const LabelHeap<Label>& heap,
                              const Region& region,
                              const std::unordered_set<Edge>& edges,
                              num minValue,
                              num maxValue,
                              OutIt outIt) const;

  template <Direction direction, class Extend>
  void setFlags(Vertex vertex,
                idx numTrees,
                Extend extend) const;

public:
  FastArcFlagPreprocessor(const Graph& graph,
                          const EdgeFunc<num>& costs,
                          const EdgeFunc<num>& deviations,
                          const Partition& partition);

  void computeFlags(RobustArcFlags& incomingFlags,
                    RobustArcFlags& outgoingFlags,
                    idx numTrees,
                    bool parallelComputation) const;

  template<class Flags>
  void computeFlags(Bidirected<Flags>& flags,
                    idx numTrees,
                    bool parallelComputation) const;
};

template<class Flags>
void FastArcFlagPreprocessor::computeFlags(Bidirected<Flags>& flags,
                                           idx numTrees,
                                           bool parallelComputation) const
{
  computeFlags(flags.get(Direction::INCOMING),
               flags.get(Direction::OUTGOING),
               numTrees,
               parallelComputation);
}


#endif /* FAST_ARCFLAG_PREPROCESSOR_HH */

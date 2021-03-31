#ifndef CENTRALIZED_PREPROCESSOR_HH
#define CENTRALIZED_PREPROCESSOR_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags.hh"
#include "partition.hh"

#include "arcflag_router.hh"

/**
 * The CentralizedPreprocessor computes ArcFlags for
 * a given Graph according to given costs and
 * a given Partition. This preprocessor employs a
 * labelling which avoids the computation of some partial
 * shortest path trees. the computation for the
 * overlapping Edge%s of a Region starts by computing
 * the trees all at once. Since most trees coincide
 * further away from their roots, this approach is
 * usually more efficient than the one employed
 * by the ArcFlagPreprocessor.
 **/
class CentralizedPreprocessor
{
private:
  template <Direction direction = Direction::OUTGOING>
  void setFlags(const Region& region);

  const Graph& graph;
  const EdgeFunc<num>& costs;
  const Partition& partition;
  ArcFlags outgoingFlags, incomingFlags;

public:
  CentralizedPreprocessor(const Graph& graph,
                          const EdgeFunc<num>& costs,
                          const Partition& partition);

  ArcFlagRouter getRouter() const;

  const ArcFlags& getOutgoingFlags() const
  {
    return outgoingFlags;
  }

  const ArcFlags& getIncomingFlags() const
  {
    return incomingFlags;
  }
};


#endif /* CENTRALIZED_PREPROCESSOR_HH */

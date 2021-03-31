#ifndef ARCFLAG_PREPROCESSOR_HH
#define ARCFLAG_PREPROCESSOR_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags.hh"
#include "arcflag_router.hh"
#include "partition.hh"

/**
 * The ArcFlagPreprocessor computes ArcFlags for
 * a given Graph according to given costs and
 * a given Partition. The preprocessor computes
 * partial shortest path trees between
 * all Edge%s which cross Region%s of the
 * given Partition and sets their respective
 * flags.
 **/
class ArcFlagPreprocessor
{
private:
  void setFlags(Direction direction);
  void setFlagsParallel(Direction direction);

  const Graph& graph;
  const EdgeFunc<num>& costs;
  const Partition& partition;
  ArcFlags outgoingFlags, incomingFlags;
  std::vector<Edge> overlappingEdges;

public:
  ArcFlagPreprocessor(const Graph& graph,
                      const EdgeFunc<num>& costs,
                      const Partition& partition,
                      bool parallel = false);

  ~ArcFlagPreprocessor();

  /**
   * Returns a Router which utilizes the computed ArcFlags
   * in order to speed up shortest path computations.
   **/
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

#endif /* ARCFLAG_PREPROCESSOR_HH */

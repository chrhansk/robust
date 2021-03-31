#ifndef BOUNDED_ARCFLAGS_HH
#define BOUNDED_ARCFLAGS_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"

#include "robust_arcflags.hh"

/**
 * The bounded arc flags store a single bit for each Edge
 * and each Region of a given Partition. The bit is set
 * iff the Edge lies on the shortest path into / out of
 * the given Region with respect to the ReducedCosts
 * defined by *some* \f$ \theta \f$. In addition, the
 * bounded arc flags store *global* lower and upper
 * bounds \f$ l_{e}, u_{e} \f$ for each Edge.
 * An Edge is required for a query with respect
 * to the ReducedCosts given by some \f$ \theta \f$
 * iff its bit with respect to the Region is set and
 * \f$ l_{e} \leq \theta \leq u_{e} \f$
 **/
class BoundedArcFlags : public RobustArcFlags
{
public:
  typedef std::vector<bool> FlagMap;

  class BoundedFlagMap
  {
  public:
    BoundedFlagMap(const Partition& partition)
      : lowerBound(inf),
        upperBound((num) -1),
        flags(partition.getRegions().size())
    {}
    num lowerBound, upperBound;
    FlagMap flags;
  };

private:
  EdgeMap<BoundedFlagMap> flagMaps;
  const Partition& partition;

public:
  BoundedArcFlags(const Graph& graph,
                  const Partition& partition);

  BoundedArcFlags(const BoundedArcFlags& other) = delete;
  BoundedArcFlags& operator=(const BoundedArcFlags& other) = delete;

  BoundedFlagMap& getFlagMap(const Edge& edge);
  const BoundedFlagMap& getFlagMap(const Edge& edge) const;

  void extend(const Edge& edge,
              const Region& region,
              num value) override;

  void extend(const Edge& edge,
              const Region& region) override;

  bool filter(const Edge& edge,
              const Region& region,
              num theta) const override;
};

#endif /* BOUNDED_ARCFLAGS_HH */

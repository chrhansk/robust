#ifndef EXTENDED_ARCFLAGS_HH
#define EXTENDED_ARCFLAGS_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"

#include "robust_arcflags.hh"

/**
 * The extended arc flags store a lower and upper bound
 * \f$ l_{e}(R), u_{e}(R) \f$ for each Edge \f$3\f$ and each
 * Region \f$ R \f$ of a given Partition.
 * An edge is required for a query into / out of a Region
 * \f$ R \f$ with respect to some \f$ \theta \f$ iff
 * \f$ l_{e}(R) \leq \theta \leq u_{e}(R) \f$.
 **/
class ExtendedArcFlags : public RobustArcFlags
{
public:
  class Entry
  {
  public:
    Entry()
      : lower(inf), upper(-1) {}
    num lower, upper;
  };

  typedef std::vector<Entry> EntryMap;

private:
  EdgeMap<EntryMap> entryMap;

public:
  ExtendedArcFlags(const Graph& graph, const Partition& partition);

  ExtendedArcFlags(const ExtendedArcFlags& other) = delete;
  ExtendedArcFlags& operator=(const ExtendedArcFlags& other) = delete;

  EntryMap& getEntryMap(const Edge& edge);
  const EntryMap& getEntryMap(const Edge& edge) const;

  void extend(const Edge& edge,
              const Region& region,
              num value) override;

  void extend(const Edge& edge,
              const Region& region) override;

  bool filter(const Edge& edge,
              const Region& region,
              num theta) const override;
};

#endif /* EXTENDED_ARCFLAGS_HH */

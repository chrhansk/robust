#ifndef ARCFLAGS_HH
#define ARCFLAGS_HH

#include <iostream>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "partition.hh"

/**
 * The ArcFlags class matains 0/1 flags with respect
 * to a given Region for each Edge of a given Graph.
 * A flag should be set iff the Edge is on a shortest
 * path into / out of the corresponding Region.
 **/
class ArcFlags
{
public:
  typedef std::vector<bool> FlagMap;

  /**
   * A filter which accepts an Edge iff the
   * flag for a provided Region is set.
   **/
  class Filter
  {
  private:
    const Region& region;
    const ArcFlags& arcFlags;
  public:
    Filter(const Region& region, const ArcFlags& arcFlags)
      : region(region), arcFlags(arcFlags) {}

    bool operator()(const Edge& edge) const
    {
      return arcFlags.hasFlag(edge, region);
    }
  };

private:
  const Graph& graph;
  const Partition& partition;
  EdgeMap<FlagMap> flagMaps;
public:
  /**
   * Constructs ArcFlags for the given Graph according
   * to the given Partition.
   **/
  ArcFlags(const Graph& graph, const Partition& partition);

  ArcFlags(const ArcFlags& other) = delete;
  ArcFlags(ArcFlags&& other);
  ArcFlags& operator=(const ArcFlags& other) = delete;

  /**
   * Sets the flag of the given Edge for the given Region.
   **/
  void setFlag(const Edge& edge, const Region& region);

  /**
   * Returns whether the flag of the given Edge with
   * respect to the given Region is set.
   **/
  bool hasFlag(const Edge& edge, const Region& region) const;

  /**
   * Returns the arc flags associated with the given Edge.
   **/
  FlagMap& getFlags(const Edge& edge);
  const FlagMap& getFlags(const Edge& edge) const;

  /**
   * Returns a FlagFilter for the given Region according
   * to these ArcFlags.
   **/
  Filter getFilter(const Region& region) const
  {
    return Filter(region, *this);
  }

  /**
   * Returns a FlagFilter for the Region containing
   * the given Vertex according to these ArcFlags.
   **/
  Filter getFilter(const Vertex& vertex) const
  {
    return Filter(partition.getRegion(vertex), *this);
  }

  const Partition& getPartition() const
  {
    return partition;
  }

  const Graph& getGraph() const
  {
    return graph;
  }

  friend std::ostream& operator<<(std::ostream&, const ArcFlags&);
};

#endif /* ARCFLAGS_HH */

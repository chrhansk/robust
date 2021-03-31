#ifndef ROBUST_ARCFLAGS_HH
#define ROBUST_ARCFLAGS_HH

#include "graph/graph.hh"
#include "arcflags/partition.hh"

/**
 * The base for all robust arc flags.
 **/
class RobustArcFlags
{
public:
  /**
   * Extends the arc flags for the given Edge to
   * be usable for the given Region and value.
   *
   **/
  virtual void extend(const Edge& edge,
                      const Region& region,
                      num value) = 0;

  /**
   * Extends the arc flags for the given Edge to
   * be usable for the given Region and *all* values.
   *
   **/
  virtual void extend(const Edge& edge,
                      const Region& region) = 0;

  /**
   * Extends the arc flags for the given Edge to
   * be usable for the given Region and the given
   * range of values.
   *
   **/
  virtual void extend(const Edge& edge,
                      const Region& region,
                      num minValue,
                      num maxValue)
  {
    extend(edge, region, minValue);
    extend(edge, region, maxValue);
  }

  /**
   * Filters an Edge according to a given Region
   * and a given value.
   *
   * @return whether or not the given Edge
   *         lies on some shortest path
   *         with respect to the ReducedCosts
   *         defined by the given value
   *         into / out of the given Region.
   *
   **/
  virtual bool filter(const Edge& edge,
                      const Region& region,
                      num value) const = 0;

  /**
   * A filter which is used in the RobustArcFlagRouter.
   **/
  class ThetaFilter
  {
  private:
    const RobustArcFlags& flags;
    const Region& region;
    const num theta;
  public:
    ThetaFilter(const RobustArcFlags& flags,
                const Region& region,
                const num theta)
      : flags(flags),
        region(region),
        theta(theta)
    {}

    /**
     * The actual filtering method. Essentially a wrapper
     * around RobustArcFlags::filter.
     **/
    bool operator()(const Edge& edge) const
    {
      return flags.filter(edge, region, theta);
    }
  };

  ThetaFilter getThetaFilter(const Region& region,
                             const num theta) const
  {
    return ThetaFilter(*this, region, theta);
  }
};


#endif /* ROBUST_ARCFLAGS_HH */

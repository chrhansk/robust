#ifndef SIMPLE_ARCFLAGS_HH
#define SIMPLE_ARCFLAGS_HH

#include "robust_arcflags.hh"

#include "arcflags/arcflags.hh"

/**
 * The simple arc flags store a single bit for each Edge
 * and each Region of a given Partition. The bit is set
 * iff the Edge lies on the shortest path into / out of
 * the given Region with respect to the ReducedCosts
 * defined by *some* \f$ \theta \f$.
 **/
class SimpleArcFlags : public RobustArcFlags,
                       public ArcFlags
{
public:
  SimpleArcFlags(const Graph& graph, const Partition& partition)
    : ArcFlags(graph, partition)
  {

  }

  SimpleArcFlags(const Graph& graph,
                 const Partition& partition,
                 const ArcFlags& arcFlags);

  SimpleArcFlags(const SimpleArcFlags& other) = delete;
  SimpleArcFlags& operator=(const SimpleArcFlags& other) = delete;

  void extend(const Edge& edge,
              const Region& region,
              num value) override;

  void extend(const Edge& edge,
              const Region& region) override;

  bool filter(const Edge& edge,
              const Region& region,
              num theta) const override;
};


#endif /* SIMPLE_ARCFLAGS_HH */

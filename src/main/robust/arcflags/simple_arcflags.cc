#include "simple_arcflags.hh"

SimpleArcFlags::SimpleArcFlags(const Graph& graph,
                               const Partition& partition,
                               const ArcFlags& arcFlags)
  : ArcFlags(graph, partition)
{
  for(const Edge& edge : getGraph().getEdges())
  {
    for(const Region& region : getPartition().getRegions())
    {
      if(arcFlags.hasFlag(edge, region))
      {
        extend(edge, region);
      }
    }
  }
}

void SimpleArcFlags::extend(const Edge& edge,
                            const Region& region,
                            num value)
{
  extend(edge, region);
}

void SimpleArcFlags::extend(const Edge& edge,
                            const Region& region)
{
  setFlag(edge, region);
}

bool SimpleArcFlags::filter(const Edge& edge,
                            const Region& region,
                            num theta) const
{
  return hasFlag(edge, region);
}

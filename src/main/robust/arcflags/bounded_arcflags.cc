#include "bounded_arcflags.hh"


BoundedArcFlags::BoundedArcFlags(const Graph& graph,
                                 const Partition& partition)
  : flagMaps(graph, BoundedFlagMap(partition)),
    partition(partition)
{

}

/*
void BoundedArcFlags::setFlag(const Edge& edge,
                              const Region& region)
{
  BoundedFlagMap& map = flagMaps(edge);
  map.flags[region.getIndex()] = true;
}
*/

BoundedArcFlags::BoundedFlagMap& BoundedArcFlags::getFlagMap(const Edge& edge)
{
  return flagMaps(edge);
}

const BoundedArcFlags::BoundedFlagMap& BoundedArcFlags::getFlagMap(const Edge& edge) const
{
  return flagMaps(edge);
}


void BoundedArcFlags::extend(const Edge& edge,
                             const Region& region,
                             num value)
{
  BoundedFlagMap& map = flagMaps(edge);
  map.flags[region.getIndex()] = true;
  map.lowerBound = std::min(map.lowerBound, value);
  map.upperBound = std::max(map.upperBound, value);
}

void BoundedArcFlags::extend(const Edge& edge,
                             const Region& region)
{
  BoundedFlagMap& map = flagMaps(edge);
  map.flags[region.getIndex()] = true;
  map.lowerBound = 0;
  map.upperBound = inf;
}


bool BoundedArcFlags::filter(const Edge& edge,
                             const Region& region,
                             num theta) const
{
  const BoundedFlagMap& flagMap = getFlagMap(edge);

  if(!flagMap.flags[region.getIndex()])
  {
    return false;
  }

  if(partition.getRegion(edge.getSource()) == region or
     partition.getRegion(edge.getTarget()) == region)
  {
    return true;
  }

  if(theta > flagMap.upperBound or
     theta < flagMap.lowerBound)
  {
    return false;
  }

  return true;
}

#include "extended_arcflags.hh"

ExtendedArcFlags::ExtendedArcFlags(const Graph& graph,
                                   const Partition& partition)
  : entryMap(graph,
             EntryMap(partition.getRegions().size(), Entry()))
{

}

ExtendedArcFlags::EntryMap&
ExtendedArcFlags::getEntryMap(const Edge& edge)
{
  return entryMap(edge);
}

const ExtendedArcFlags::EntryMap&
ExtendedArcFlags::getEntryMap(const Edge& edge) const
{
  return entryMap(edge);
}

void ExtendedArcFlags::extend(const Edge& edge,
                              const Region& region,
                              num value)
{
  Entry& entry = getEntryMap(edge)[region.getIndex()];
  entry.lower = std::min(entry.lower, value);
  entry.upper = std::max(entry.upper, value);
}

void ExtendedArcFlags::extend(const Edge& edge,
                              const Region& region)
{
  Entry& entry = getEntryMap(edge)[region.getIndex()];
  entry.lower = 0;
  entry.upper = inf;
}

bool ExtendedArcFlags::filter(const Edge& edge,
                              const Region& region,
                              num theta) const
{
  const EntryMap& flagMap = getEntryMap(edge);

  if(theta < flagMap[region.getIndex()].lower)
  {
    return false;
  }

  if(theta > flagMap[region.getIndex()].upper)
  {
    return false;
  }

  return true;
}

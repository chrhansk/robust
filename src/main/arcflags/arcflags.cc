#include "arcflags.hh"

ArcFlags::ArcFlags(const Graph& graph,
                   const Partition& partition)
  : graph(graph),
    partition(partition),
    flagMaps(graph, FlagMap(partition.getRegions().size()))
{
}

ArcFlags::ArcFlags(ArcFlags&& other)
  : graph(other.graph),
    partition(other.partition),
    flagMaps(std::move(other.flagMaps))
{
}

void ArcFlags::setFlag(const Edge& edge, const Region& region)
{
  FlagMap& map = flagMaps(edge);
  map[region.getIndex()] = true;
}

ArcFlags::FlagMap& ArcFlags::getFlags(const Edge& edge)
{
  return flagMaps(edge);
}

const ArcFlags::FlagMap& ArcFlags::getFlags(const Edge& edge) const
{
  return flagMaps(edge);
}

bool ArcFlags::hasFlag(const Edge& edge, const Region& region) const
{
  return flagMaps(edge)[region.getIndex()];
}


std::ostream& operator<<(std::ostream& out, const ArcFlags& flags)
{
  int c = 0;

  for(const Edge& edge : flags.graph.getEdges())
  {
    for(const Region& region : flags.partition.getRegions())
    {
      if(flags.hasFlag(edge, region))
      {
        ++c;
      }
    }
  }

  const int size = flags.partition.getRegions().size();

  out << "Flags of size " << size
      << ", average utilization: "
      << (c /((float) flags.graph.getEdges().size() * size));

  return out;
}

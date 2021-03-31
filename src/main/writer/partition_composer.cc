#include "partition_composer.hh"

#include "log.hh"

void
PartitionComposer::composePartition(const Partition& partition,
                                    Protobuf::Partition& PBFPartition)
{
  Log(info) << "Composing partition";

  if(!partition.isValid())
  {
    throw std::runtime_error("Attempting to compose an invalid partition");
  }

  for(const Region& region : partition.getRegions())
  {
    Protobuf::Region& PBFRegion = *(PBFPartition.add_regions());

    for(const Vertex& vertex : region.getVertices())
    {
      PBFRegion.add_vertices(vertex.getIndex());
    }
  }
}

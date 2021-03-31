#include "partition_parser.hh"

#include "log.hh"

std::unique_ptr<Partition> PartitionParser::parsePartition(const Graph& graph,
                                                           const Protobuf::Partition& PBFPartition)
{
  std::unique_ptr<Partition> partition(new Partition(graph));

  Log(info) << "Parsing partition";

  std::vector<Vertex> vertices = graph.getVertices().collect();

  for(int i = 0; i < PBFPartition.regions_size();++i)
  {
    const Protobuf::Region &PBFRegion = PBFPartition.regions(i);
    std::vector<Vertex> regionVertices;

    for(int j = 0; j < PBFRegion.vertices_size(); ++j)
    {
      regionVertices.push_back(vertices[PBFRegion.vertices(j)]);
    }

    partition->addRegion(regionVertices);
  }

  if(!partition->isValid())
  {
    throw std::runtime_error("Read in an invalid partition");
  }

  return partition;
}

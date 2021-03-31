#include "arcflag_parser.hh"

#include "log.hh"

#include "arcflags/arcflags.hh"

const idx batchSize = 32;

std::unique_ptr<ArcFlags>
ArcFlagParser::parseArcFlags(const Graph& graph,
                             const Partition& partition,
                             const Protobuf::ArcFlags& PBFArcFlags)
{
  Log(info) << "Parsing arc flags";

  std::unique_ptr<ArcFlags> arcFlags(new ArcFlags(graph, partition));

  const idx numRegions = partition.getRegions().size();

  if((numRegions % batchSize) != 0)
  {
    throw std::runtime_error("Partition must contain of a multiple of 32 regions");
  }

  const idx numBatches = numRegions / batchSize;

  if(PBFArcFlags.flags_size() != (int) (graph.getEdges().size() * numBatches))
  {
    throw std::runtime_error("Message has an invalid length");
  }

  idx currentIndex = 0;

  for(const Edge& edge : graph.getEdges())
  {
    auto it = partition.getRegions().begin();

    for(idx currentBatch = 0; currentBatch != numBatches; ++currentBatch)
    {
      google::protobuf::uint32 flags = PBFArcFlags.flags(currentIndex++);
      for(idx batchIndex = 0; batchIndex != batchSize; ++batchIndex)
      {
        if(flags & (1 << batchIndex))
        {
          arcFlags->setFlag(edge, *(it));
        }

        ++it;
      }
    }

    assert(it == partition.getRegions().end());
  }

  Log(info) << "Parsed arc flags";

  return arcFlags;
}

#include "arcflag_composer.hh"

#include "log.hh"

const idx batchSize = 32;

void
ArcFlagComposer::composeArcFlags(const ArcFlags& arcFlags,
                                 Protobuf::ArcFlags& PBFArcFlags)
{
  Log(info) << "Composing arc flags";

  const Partition& partition = arcFlags.getPartition();
  const Graph& graph = partition.getGraph();

  const idx numRegions = partition.getRegions().size();

  if((numRegions % batchSize) != 0)
  {
    throw std::runtime_error("Partition must contain of a multiple of 32 regions");
  }

  const idx numBatches = numRegions / batchSize;

  for(const Edge& edge : graph.getEdges())
  {
    auto it = partition.getRegions().begin();

    for(idx currentBatch = 0; currentBatch != numBatches; ++currentBatch)
    {
      google::protobuf::uint32 flags = 0;
      for(idx batchIndex = 0; batchIndex != batchSize; ++batchIndex)
      {
        if(arcFlags.hasFlag(edge, *(it++)))
        {
          flags |= (1 << batchIndex);
        }
      }

      PBFArcFlags.add_flags(flags);
    }

    assert(it == partition.getRegions().end());
  }

  Log(info) << "Composed arc flags";
}

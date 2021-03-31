#include "required_values_writer.hh"

#include "graph.pb.h"

#include "partition_composer.hh"

void
RequiredValuesWriter::writeRequiredValues(std::ostream& out,
                                          const EdgeFunc<num>& costs,
                                          const EdgeFunc<num>& deviations,
                                          idx deviationSize,
                                          const Partition& partition,
                                          const RegionPairMap<ValueVector>& requiredValues)
{
  using namespace google::protobuf::io;

  Protobuf::RequiredValues PBFValues;

  if(!partition.isValid())
  {
    throw std::runtime_error("Attempting to save an invalid partition");
  }

  Protobuf::IntTagValues& PBFCosts = *(PBFValues.mutable_costs());
  Protobuf::IntTagValues& PBFDeviations = *(PBFValues.mutable_deviations());

  for(const Edge& edge : partition.getGraph().getEdges())
  {
    if(costs(edge) < 0)
    {
      throw std::runtime_error("Attempting to save invalid cost value");
    }

    if(deviations(edge) < 0)
    {
      throw std::runtime_error("Attempting to save invalid deviation value");
    }

    PBFCosts.add_values(costs(edge));
    PBFDeviations.add_values(deviations(edge));
  }

  PartitionComposer().composePartition(partition,*(PBFValues.mutable_partition()));

  for(const Region& sourceRegion : partition.getRegions())
  {
    for(const Region& targetRegion : partition.getRegions())
    {
      if(sourceRegion == targetRegion)
      {
        continue;
      }

      ValueVector values = requiredValues(sourceRegion, targetRegion);

      std::sort(values.begin(), values.end(), std::greater<num>());

      Protobuf::ValueVector& PBFValueVector = *(PBFValues.add_values());

      for(const num& value : values)
      {
        PBFValueVector.add_values(value);
      }
    }
  }

  PBFValues.set_deviation_size(deviationSize);

  PBFValues.SerializeToOstream(&out);
}

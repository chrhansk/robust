#include "required_values_reader.hh"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "log.hh"

#include "graph.pb.h"

#include "arcflags/region_pair_map.hh"
#include "robust/robust_utils.hh"

#include "partition_parser.hh"

const int maxSize = std::numeric_limits<int32_t>::max();

RequiredValuesReadResult
RequiredValuesReader::readRequiredValues(const Graph& graph,
                                         std::istream& in)
{
  using namespace google::protobuf::io;

  Protobuf::RequiredValues PBFValues;

  Log(info) << "Reading required values";

  IstreamInputStream* input = new IstreamInputStream(&in);
  CodedInputStream* codedInput = new CodedInputStream(input);

  codedInput->SetTotalBytesLimit(maxSize, maxSize);

  if(!in)
  {
    throw std::runtime_error("Could not open input");
  }
  else if(!PBFValues.MergeFromCodedStream(codedInput))
  {
    throw std::runtime_error("Failed to parse input");
  }

  EdgeMap<num> costs(graph, 0), deviations(graph, 0);

  const Protobuf::IntTagValues& PBFCosts = PBFValues.costs();
  const Protobuf::IntTagValues& PBFDeviations = PBFValues.deviations();

  {
    int i = 0;
    for(const Edge& edge : graph.getEdges())
    {
      costs(edge) = PBFCosts.values(i);
      deviations(edge) = PBFDeviations.values(i);
      ++i;
    }
  }

  idx deviationSize = PBFValues.deviation_size();

  std::unique_ptr<Partition> partition = PartitionParser().parsePartition(graph, PBFValues.partition());

  Log(info) << "Read in a partition of size " << partition->getRegions().size();

  std::unique_ptr<RegionPairMap<ValueVector>> requiredValues(new RegionPairMap<ValueVector>());

  idx c = 0;
  for(idx i = 0; i < partition->getRegions().size(); ++i)
  {
    const Region& sourceRegion = partition->getRegions()[i];
    for(idx j = 0; j < partition->getRegions().size(); ++j)
    {
      if(i == j)
      {
        continue;
      }

      const Region& targetRegion = partition->getRegions()[j];

      ValueVector values;

      const Protobuf::ValueVector& PBFCurrentValues = PBFValues.values(c);

      for(int k = 0; k < PBFCurrentValues.values_size(); ++k)
      {
        values.push_back(PBFCurrentValues.values(k));
      }

      std::sort(values.begin(), values.end(), std::greater<num>());

      requiredValues->put(sourceRegion, targetRegion, values);

      ++c;
    }
  }

  Log(info) << "Read in " << c << " vectors of values";

  return RequiredValuesReadResult(costs, deviations, deviationSize,
                                  std::move(partition),
                                  std::move(requiredValues));
}

#include <iostream>
#include <fstream>

#include <tbb/tbb.h>

#include "util.hh"

#include "log.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "reader/graph_reader.hh"
#include "reader/required_values_reader.hh"

#include "robust/robust_utils.hh"

#include "arcflags/metis_partition.hh"

#include "robust/values/refining_value_preprocessor.hh"

#include "robust/arcflags/simple_arcflags.hh"
#include "robust/arcflags/value_arcflag_preprocessor.hh"

#include "writer/bidirected_arcflag_writer.hh"

int main(int argc, char **argv)
{
  logInit();

  if(argc != 4)
  {
    std::cerr << "Usage: "
              << argv[0]
              << " <graphfile> <valuefile> <flagfile>"
              << std::endl;

    return 1;
  }

  std::fstream graphInput(argv[1], std::ios_base::in | std::ios_base::binary);

  ReadResult result = GraphReader().readGraph(graphInput);

  const Graph& graph = result.graph;

  std::fstream valuesInput(argv[2], std::ios_base::in | std::ios_base::binary);

  RequiredValuesReadResult valuesResult = RequiredValuesReader().readRequiredValues(graph,
                                                                                    valuesInput);

  const Partition& partition = *(valuesResult.partition);

  ValueVector values = thetaValues(graph, result.deviations.getValues());
  long totalValues = 0, actualValues = 0, regionPairs = 0;

  for(const Region& sourceRegion : partition.getRegions())
  {
    for(const Region& targetRegion : partition.getRegions())
    {
      if(sourceRegion == targetRegion)
      {
        continue;
      }

      ++regionPairs;

      totalValues += values.size();
      actualValues += (*valuesResult.requiredValues)(sourceRegion, targetRegion).size();
    }
  }

  Log(info) << "Average number of required values: "
            << actualValues / ((float) regionPairs)
            << ", number of potential values: "
            << totalValues / ((float) regionPairs)
            << ", average percentage: "
            << 100*(actualValues / ((float) totalValues));

  Log(info) << "Computing arc flags";

  auto costs = valuesResult.costs.getValues();
  auto deviations = valuesResult.deviations.getValues();

  ValueArcFlagPreprocessor preprocessor(graph, costs, deviations, partition);
  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, *(valuesResult.requiredValues), true);

  std::ofstream out(argv[3]);

  BidirectedArcFlagWriter().writeBidirectedArcFlags(out,
                                                    flags.get<Direction::INCOMING>(),
                                                    flags.get<Direction::OUTGOING>());

  return 0;
}

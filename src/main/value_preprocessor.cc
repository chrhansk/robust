#include <iostream>
#include <fstream>

#include <tbb/tbb.h>

#include "util.hh"

#include "log.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "reader/graph_reader.hh"

#include "writer/required_values_writer.hh"

#include "arcflags/metis_partition.hh"

#include "robust/values/refining_value_preprocessor.hh"

int main(int argc, char **argv)
{
  logInit();

  if(argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <graphfile> <valuefile>";
    return 1;
  }

  std::fstream input(argv[1], std::ios_base::in | std::ios_base::binary);

  ReadResult result = GraphReader().readGraph(input);

  const Graph& graph = result.graph;

  METISPartition partition(graph, 64);

  EdgeValueMap<num> costs = result.costs.getValues();
  EdgeValueMap<num> deviations = result.deviations.getValues();
  const num deviationSize = 5;

  RegionPairMap<ValueVector> requiredValues;

  tbb::spin_mutex mutex;

  tbb::parallel_do(partition.getRegions().begin(),
                   partition.getRegions().end(),
                   [&](const Region& sourceRegion)
                   {
                     Log(info) << "Computing required values leaving a region of size "
                               << sourceRegion.getVertices().size();

                     RefiningValuePreprocessor preprocessor(graph,
                                                            costs,
                                                            deviations,
                                                            deviationSize,
                                                            partition);

                     RegionMap<ValueSet> nextValues = preprocessor.requiredValues(sourceRegion);

                     {
                       tbb::spin_mutex::scoped_lock lock(mutex);

                       for(const Region& targetRegion : partition.getRegions())
                       {
                         if(sourceRegion == targetRegion)
                         {
                           continue;
                         }

                         const ValueSet& currentValues = nextValues(targetRegion);

                         ValueVector values(currentValues.begin(), currentValues.end());

                         requiredValues.put(sourceRegion, targetRegion, values);
                       }
                     }

                   });

  std::ofstream output(argv[2]);

  RequiredValuesWriter().writeRequiredValues(output,
                                             costs,
                                             deviations,
                                             deviationSize,
                                             partition,
                                             requiredValues);

  return 0;
}

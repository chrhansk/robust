#include "value_arcflag_preprocessor.hh"


ValueArcFlagPreprocessor::ValueArcFlagPreprocessor(const Graph& graph,
                                                   const EdgeFunc<num>& costs,
                                                   const EdgeFunc<num>& deviations,
                                                   const Partition& partition)
  : AbstractArcFlagPreprocessor(graph, costs, deviations, partition)
{

}

void
ValueArcFlagPreprocessor::computeFlags(RobustArcFlags& incomingFlags,
                                       RobustArcFlags& outgoingFlags,
                                       const RegionPairMap<ValueVector>& possibleValues,
                                       bool parallelComputation) const
{
  if(parallelComputation)
  {
    tbb::spin_mutex incomingMutex, outgoingMutex;

    tbb::parallel_do(partition.getRegions().begin(),
                     partition.getRegions().end(),
                     [&](const Region& region)
                     {
                       computeFlagsParallel<Direction::OUTGOING>(region,
                                                                 possibleValues,
                                                                 outgoingFlags,
                                                                 outgoingMutex);

                       computeFlagsParallel<Direction::INCOMING>(region,
                                                                 possibleValues,
                                                                 incomingFlags,
                                                                 incomingMutex);
                     }
      );

  }
  else
  {
    for(const Region& region : partition.getRegions())
    {
      computeFlags<Direction::OUTGOING>(region, possibleValues, outgoingFlags);
      computeFlags<Direction::INCOMING>(region, possibleValues, incomingFlags);
    }
  }
}


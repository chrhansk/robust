#ifndef REQUIRED_VALUES_READER_H
#define REQUIRED_VALUES_READER_H

#include <iostream>
#include <memory>

#include "graph/graph.hh"
#include "graph/edge_map.hh"

#include "arcflags/partition.hh"
#include "arcflags/region_pair_map.hh"

#include "robust/robust_utils.hh"

class RequiredValuesReadResult
{
public:
  RequiredValuesReadResult(const EdgeMap<num>& costs,
                           const EdgeMap<num>& deviations,
                           idx deviationSize,
                           std::unique_ptr<Partition> partition,
                           std::unique_ptr<RegionPairMap<ValueVector>> requiredValues)
    : costs(costs),
      deviations(deviations),
      deviationSize(deviationSize),
      partition(std::move(partition)),
      requiredValues(std::move(requiredValues))
  {}

  EdgeMap<num> costs;
  EdgeMap<num> deviations;
  idx deviationSize;
  std::unique_ptr<Partition> partition;
  std::unique_ptr<RegionPairMap<ValueVector>> requiredValues;
};

class RequiredValuesReader
{
public:
  RequiredValuesReadResult readRequiredValues(const Graph& graph,
                                              std::istream& in);
};


#endif /* REQUIRED_VALUES_READER_H */

#ifndef PBF_REQUIRED_VALUES_WRITER_H
#define PBF_REQUIRED_VALUES_WRITER_H

#include <iostream>

#include "arcflags/region_pair_map.hh"

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "arcflags/partition.hh"
#include "robust/robust_utils.hh"

class RequiredValuesWriter
{
public:
  void writeRequiredValues(std::ostream& out,
                           const EdgeFunc<num>& costs,
                           const EdgeFunc<num>& deviations,
                           idx deviationSize,
                           const Partition& partition,
                           const RegionPairMap<ValueVector>& requiredValues);
};


#endif /* PBF_REQUIRED_VALUES_WRITER_H */

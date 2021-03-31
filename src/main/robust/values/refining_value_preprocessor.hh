#ifndef REFINING_VALUE_PREPROCESSOR_HH
#define REFINING_VALUE_PREPROCESSOR_HH

#include "arcflags/region_map.hh"

#include "abstract_value_preprocessor.hh"

/**
 * The RefiningValuePreprocessor computes a superset of the required
 * values in two steps:
 *
 * 1. A set of possible values is determined by computing
 *    all shortest paths between boundary vertices and
 *    collecting all values occurring on these paths
 *    as well as all values occurring in the regions
 *    themselves.
 *
 * 2. The possible values are filtered by a OuterValuePreprocessor
 *
 **/
class RefiningValuePreprocessor : public AbstractValuePreprocessor
{
public:
  RefiningValuePreprocessor(const Graph& graph,
                            const EdgeFunc<num>& costs,
                            const EdgeFunc<num>& deviations,
                            num deviationSize,
                            const Partition& partition)
    : AbstractValuePreprocessor(graph,
                                costs,
                                deviations,
                                deviationSize,
                                partition)
  {}


  ValueSet requiredValues(const Region& sourceRegion,
                          const Region& targetRegion,
                          const ValueSet& possibleValues) const override;

  RegionMap<ValueSet>
  requiredValues(const Region& sourceRegion) const;

  using AbstractValuePreprocessor::requiredValues;
};


#endif /* REFINING_VALUE_PREPROCESSOR_HH */

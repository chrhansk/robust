#ifndef OUTER_VALUE_PREPROCESSOR_HH
#define OUTER_VALUE_PREPROCESSOR_HH

#include "abstract_value_preprocessor.hh"

/**
 * The OuterValuePreprocessor computes a superset
 * of the required values by considering the
 * boundary vertices of a pair of regions.
 * The minimal values for the boundary vertices are
 * added to the required values. For all other
 * values it is determined whether these values
 * could improve the cost of a path. If this
 * is the case, the value is added to the required
 * values as well.
 **/
class OuterValuePreprocessor : public AbstractValuePreprocessor
{
public:
  OuterValuePreprocessor(const Graph& graph,
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

  ValueSet requiredValues(const Region& sourceRegion,
                          const Region& targetRegion,
                          const DistanceMap& boundaryDistances,
                          const ValueSet& possibleValues) const;

  using AbstractValuePreprocessor::requiredValues;

};


#endif /* OUTER_VALUE_PREPROCESSOR_HH */

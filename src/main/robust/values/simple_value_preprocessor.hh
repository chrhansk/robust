#ifndef SIMPLE_VALUE_PREPROCESSOR_HH
#define SIMPLE_VALUE_PREPROCESSOR_HH

#include <unordered_set>

#include "abstract_value_preprocessor.hh"

class SimpleValuePreprocessor : public AbstractValuePreprocessor
{
public:
  SimpleValuePreprocessor(const Graph& graph,
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

  std::unordered_set<num> requiredValues(const Region& sourceRegion,
                                         const Region& targetRegion) const;

};


#endif /* SIMPLE_VALUE_PREPROCESSOR_HH */

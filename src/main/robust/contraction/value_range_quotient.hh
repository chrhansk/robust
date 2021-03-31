#ifndef VALUE_RANGE_QUOTIENT_HH
#define VALUE_RANGE_QUOTIENT_HH

#include "robust_score_function.hh"
#include "robust_contraction_preprocessor.hh"

class ValueRangeQuotient : public RobustScoreFunction
{
public:
  ValueRangeQuotient(const Graph& graph,
                     const EdgeFunc<const ContractionRange&>& contractionRanges)
    : RobustScoreFunction(graph, contractionRanges)
  {}

  virtual float getScore(Vertex vertex,
                         const std::vector<RobustContractionPair>& pairs) override;
};


#endif /* VALUE_RANGE_QUOTIENT_HH */

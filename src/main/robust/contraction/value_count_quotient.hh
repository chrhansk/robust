#ifndef VALUE_COUNT_QUOTIENT_HH
#define VALUE_COUNT_QUOTIENT_HH

#include "robust_score_function.hh"
#include "robust_contraction_preprocessor.hh"

class ValueCountQuotient : public RobustScoreFunction
{
public:
  ValueCountQuotient(const Graph& graph,
                     const EdgeFunc<const ContractionRange&>& contractionRanges)
    : RobustScoreFunction(graph, contractionRanges)
  {}

  virtual float getScore(Vertex vertex,
                         const std::vector<RobustContractionPair>& pairs) override;
};


#endif /* VALUE_COUNT_QUOTIENT_HH */

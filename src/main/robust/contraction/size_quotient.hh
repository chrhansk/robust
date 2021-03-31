#ifndef SIZE_QUOTIENT_HH
#define SIZE_QUOTIENT_HH

#include "robust_score_function.hh"
#include "robust_contraction_preprocessor.hh"

class SizeQuotient : public RobustScoreFunction
{
private:
  const RobustContractionPreprocessor& preprocessor;
public:
  SizeQuotient(const RobustContractionPreprocessor& preprocessor)
    : preprocessor(preprocessor)
  {}

  virtual float getScore(Vertex vertex,
                         const std::vector<RobustContractionPair>& pairs) override;
};


#endif /* SIZE_QUOTIENT_HH */

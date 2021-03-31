#ifndef CONTRACTION_PREPROCESSOR_HH
#define CONTRACTION_PREPROCESSOR_HH

#include "abstract_contraction_preprocessor.hh"

class ContractionPreprocessor : public AbstractContractionPreprocessor
{
public:
  ContractionPreprocessor(const Graph& graph,
                          const EdgeFunc<num>& costs);

  ContractionHierarchy computeHierarchy() override;
};

#endif /* CONTRACTION_PREPROCESSOR_HH */

#ifndef ROBUST_CONTRACTION_PREPROCESSOR_HH
#define ROBUST_CONTRACTION_PREPROCESSOR_HH

#include <vector>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "robust/robust_utils.hh"

#include "contraction_range.hh"

#include "abstract_robust_contraction_preprocessor.hh"
#include "robust_contraction_hierarchy.hh"
#include "robust_contraction_pair.hh"

class RobustContractionPreprocessor : public AbstractRobustContractionPreprocessor
{
public:
  RobustContractionPreprocessor(const Graph& graph,
                                const EdgeFunc<num>& costs,
                                const EdgeFunc<num>& deviations);

  RobustContractionHierarchy computeHierarchy() const override;
};

#endif /* ROBUST_CONTRACTION_PREPROCESSOR_HH */

#ifndef ROBUST_SEARCH_PREDICATE_HH
#define ROBUST_SEARCH_PREDICATE_HH

#include "graph/graph.hh"
#include "robust_contraction_preprocessor.hh"

class RobustSearchPredicate
{
private:
  const VertexFunc<bool>& contracted;
  const EdgeFunc<const ContractionRange&>& contractionRanges;
  Vertex vertex;
  num thetaValue;
public:
  RobustSearchPredicate(const VertexFunc<bool>& contracted,
                        const EdgeFunc<const ContractionRange&>& contractionRanges,
                        Vertex vertex,
                        num thetaValue)
    : contracted(contracted),
      contractionRanges(contractionRanges),
      vertex(vertex),
      thetaValue(thetaValue)
  {}

  bool operator()(const Edge& edge) const;
};

#endif /* ROBUST_SEARCH_PREDICATE_HH */

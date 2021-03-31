#include "robust_search_predicate.hh"


bool RobustSearchPredicate::operator()(const Edge& edge) const
{
  const Vertex& source = edge.getSource();
  const Vertex& target = edge.getTarget();

  const ContractionRange& range = contractionRanges(edge);

  return !contracted(source) and
    !contracted(target) and
    source != vertex and
    target != vertex and
    thetaValue <= range.getMaximum() and
    thetaValue >= range.getMinimum();
}

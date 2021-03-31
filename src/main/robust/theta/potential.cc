#include "potential.hh"

bool Potential::isValidFor(const EdgeFunc<num>& costs) const
{
  for(const Edge& edge : graph.getEdges())
  {
    Vertex source = edge.getSource();
    Vertex target = edge.getTarget();

    num sourceVal = (*this)(source);
    num targetVal = (*this)(target);

    if(sourceVal == targetVal)
    {
      continue;
    }

    if(sourceVal == inf or targetVal == inf)
    {
      return false;
    }

    if(costs(edge) - sourceVal + targetVal < 0)
    {
      return false;
    }

  }
  return true;
}

bool Potential::isTightFor(const EdgeFunc<num>& costs,
                           const Edge& edge) const
{
  const Vertex source = edge.getSource();
  const Vertex target = edge.getTarget();

  const num sourceVal = (*this)(source);
  const num targetVal = (*this)(target);

  const num cost = costs(edge);

  if(sourceVal == inf or targetVal == inf)
  {
    return false;
  }

  if(cost - sourceVal + targetVal == 0)
  {
    return true;
  }

  return false;
}

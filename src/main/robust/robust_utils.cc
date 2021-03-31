#include "robust_utils.hh"

#include <algorithm>
#include <cmath>

template class std::vector<num>;
template class std::vector<ValueVector>;

ValueVector thetaValues(const Graph& graph,
                             const EdgeFunc<num>& deviations)
{
  ValueVector values;

  for(Edge edge : graph.getEdges())
  {
    values.push_back(deviations(edge));
  }

  std::sort(values.begin(), values.end(), std::greater<num>());

  auto it = std::unique(values.begin(), values.end());

  num size = std::distance(values.begin(), it);

  if(values[size -1] == 0)
  {
    values.resize(size);
    return values;
  }

  values.resize(size + 1);
  values[values.size() - 1] = (num) 0;

  return values;
}

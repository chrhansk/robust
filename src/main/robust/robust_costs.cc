#include "robust_costs.hh"

#include <algorithm>
#include <iostream>
#include <queue>

#include "reduced_costs.hh"

RobustCosts::RobustCosts(const EdgeFunc<num>& costs,
                         const EdgeFunc<num>& deviations,
                         idx deviationSize)
  : costs(costs),
    deviations(deviations),
    deviationSize(deviationSize)
{

}


num RobustCosts::get(const Path& path)
{
  num sum = (num) 0;

  const std::deque<Edge>& pathEdges = path.getEdges();
  std::vector<Edge> edges(pathEdges.begin(), pathEdges.end());

  for(const Edge& edge : edges)
  {
    sum += costs(edge);
  }

  if(edges.size() <= deviationSize)
  {
    for(const Edge& edge : edges)
    {
      sum += deviations(edge);
    }
  }
  else
  {
    std::vector<num> values;

    for(const Edge& edge : edges)
    {
      values.push_back(deviations(edge));
    }

    std::sort(values.begin(), values.end());

    auto it = values.rbegin();

    for(idx i = 0; i < deviationSize; ++i)
    {
      sum += *it;
      ++it;
    }

    /*
    auto comp = [this](const Edge& first, const Edge& second) -> bool
      {
        return deviations(first) < deviations(second);
      };

    std::priority_queue<Edge,
                        std::vector<Edge>,
                        decltype(comp)> edgeQueue(comp, edges);

    for(int i = 0; i < deviationSize; ++i)
    {
      sum += deviations(edgeQueue.top());
      edgeQueue.pop();
    }
    */
  }

  return sum;
}


num RobustCosts::optimalCost(const Path& path,
                             const ValueVector& values)
{
  auto pathCost = [&](num value) -> num
    {
      return deviationSize*value + path.cost(ReducedCosts(costs, deviations, value));
    };

  auto it = std::min(values.begin(), values.end(),
                     [&](ValueIterator first, ValueIterator second) -> bool
    {
      return pathCost(*first) < pathCost(*second);
    });

  return *it;
}

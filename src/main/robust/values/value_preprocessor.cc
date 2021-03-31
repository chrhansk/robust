#include "value_preprocessor.hh"

#include <unordered_map>

#include "log.hh"

#include "robust/robust_utils.hh"
#include "robust/theta/theta_router.hh"

struct Bound
{
  num value;
  num cost;

  Bound()
    : value(-1),
      cost(inf)
  {}

  Bound(num value, num cost)
    : value(value),
      cost(cost)
  {}
};

std::unordered_set<num>
ValuePreprocessor::requiredValues(const Region& sourceRegion,
                                  const Region& targetRegion) const
{
  std::unordered_set<num> requiredValues;

  ValueVector values = thetaValues(graph, deviations);

  auto sourceBoundaries =
    partition.boundaryVertices<Direction::OUTGOING>(sourceRegion);

  auto targetBoundaries =
    partition.boundaryVertices<Direction::INCOMING>(targetRegion);

  DistanceMap boundaryDistances = findShortestPaths(sourceRegion, targetRegion);

  DistanceMap sourceDistances, targetDistances;

  Log(info) << "Computing shortest paths within source region";

  for(const Vertex& source : sourceRegion.getVertices())
  {
    for(const Vertex& sourceBoundary : sourceBoundaries)
    {
      ValueVector currentValues;
      currentValues.reserve(values.size());
      sourceDistances.put(source, sourceBoundary, std::move(currentValues));
    }
  }

  computeShortestPaths<Direction::OUTGOING>(sourceRegion,
                                            sourceBoundaries,
                                            [&](const Vertex& vertex,
                                                const Vertex& boundaryVertex,
                                                num value)
                                            {
                                              sourceDistances(vertex,
                                                              boundaryVertex)
                                                .push_back(value);
                                            });

  for(const Vertex& target : targetRegion.getVertices())
  {
    for(const Vertex& targetBoundary : targetBoundaries)
    {
      ValueVector currentValues;
      currentValues.reserve(values.size());
      targetDistances.put(targetBoundary, target, std::move(currentValues));
    }
  }

  computeShortestPaths<Direction::INCOMING>(targetRegion,
                                            targetBoundaries,
                                            [&](const Vertex& boundaryVertex,
                                                const Vertex& vertex,
                                                num value)
                                            {
                                              targetDistances(boundaryVertex,
                                                              vertex)
                                                .push_back(value);
                                            });

  Log(info) << "Performing distance lookups";

  for(const Vertex& source : sourceRegion.getVertices())
  {
    for(const Vertex& target : targetRegion.getVertices())
    {
      num bestCost = inf;
      num bestValue = -1;

      for(const Vertex& sourceBoundary : sourceBoundaries)
      {
        const ValueVector& sourceDistanceValues =
          sourceDistances(source, sourceBoundary);

        if(*sourceDistanceValues.begin() == inf)
        {
          continue;
        }

        for(const Vertex& targetBoundary : targetBoundaries)
        {
          const ValueVector& targetDistanceValues =
            targetDistances(targetBoundary, target);

          const ValueVector& boundaryDistanceValues =
            boundaryDistances(sourceBoundary, targetBoundary);

          if(*targetDistanceValues.begin() == inf)
          {
            continue;
          }

          for(idx i = 0; i < values.size(); ++i)
          {
            const num& value = values[i];

            const num sourceDistance = sourceDistanceValues[i];
            const num boundaryDistance = boundaryDistanceValues[i];
            const num targetDistance = targetDistanceValues[i];

            if(boundaryDistance == inf or
               sourceDistance == inf or
               targetDistance == inf)
            {
              continue;
            }

            num currentCost = sourceDistance + boundaryDistance + targetDistance;

            if(currentCost > bestCost)
            {
              break;
            }

            currentCost += (value * deviationSize);

            if(currentCost < bestCost)
            {
              bestCost = currentCost;
              bestValue = value;
            }
          }
        }
      }

      if(bestValue != -1)
      {
        requiredValues.insert(bestValue);
      }
    }
  }

  return requiredValues;
}

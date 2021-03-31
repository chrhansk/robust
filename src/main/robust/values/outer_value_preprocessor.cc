#include "outer_value_preprocessor.hh"

#include "log.hh"

#include "router/distance_tree.hh"
#include "robust/reduced_costs.hh"

#include "vertex_pair_map.hh"

ValueSet
OuterValuePreprocessor::requiredValues(const Region& sourceRegion,
                                       const Region& targetRegion,
                                       const DistanceMap& boundaryDistances,
                                       const ValueSet& possibleValues) const
{
  Log(info) << "Computing additional required values.";

  ValueSet requiredValues;

  Boundary sourceBoundary = partition.getBoundary<Direction::OUTGOING>(sourceRegion);
  Boundary targetBoundary = partition.getBoundary<Direction::INCOMING>(targetRegion);

  auto sourceFilter = partition.regionFilter(sourceRegion);
  auto targetFilter = partition.regionFilter(targetRegion);

  for(const Vertex& source : sourceBoundary.getVertices())
  {
    for(const Vertex& target : targetBoundary.getVertices())
    {
      if(boundaryDistances(source, target).empty())
      {
        continue;
      }

      num bestValue = -1;
      num bestCost = inf;
      idx bestIndex = 0;

      Log(debug) << "Optimizing between boundary vertices.";

      for(idx i = 0; i < values.size(); ++i)
      {
        num value = values[i];

        if(possibleValues.find(value) == possibleValues.end())
        {
          continue;
        }

        num currentCost = value*deviationSize + boundaryDistances(source, target)[i];

        if(currentCost < bestCost)
        {
          bestCost = currentCost;
          bestValue = value;
          bestIndex = i;
        }
      }

      ReducedCosts bestReducedCosts(costs, deviations, bestValue);
      DistanceTree<Direction::INCOMING> sourceBounds(graph, bestReducedCosts);
      DistanceTree<Direction::OUTGOING> targetBounds(graph, bestReducedCosts);

      requiredValues.insert(bestValue);

      Log(debug) << "Computing upper bounds between vertices.";

      // compute upper bounds
      {
        sourceBounds.add(source);
        sourceBounds.extend(sourceRegion.getVertices().begin(),
                            sourceRegion.getVertices().end(),
                            sourceFilter);

        targetBounds.add(target);
        targetBounds.extend(targetRegion.getVertices().begin(),
                            targetRegion.getVertices().end(),
                            targetFilter);
      }

      Log(debug) << "Computing lower bounds between vertices.";
      idx numValues = 0;

      for(idx i = 0; i < values.size(); ++i)
      {
        const num& value = values[i];

        if(possibleValues.find(value) == possibleValues.end())
        {
          continue;
        }

        if(value <= bestValue or
           (requiredValues.find(value) != requiredValues.end()))
        {
          continue;
        }

        ++numValues;

        ReducedCosts reducedCosts(costs, deviations, value);

        DistanceTree<Direction::INCOMING> sourceDistances(graph, reducedCosts);
        DistanceTree<Direction::OUTGOING> targetDistances(graph, reducedCosts);

        {
          sourceDistances.add(source);
          sourceDistances.extend(sourceRegion.getVertices().begin(),
                                 sourceRegion.getVertices().end(),
                                 sourceFilter);

          targetDistances.add(target);
          targetDistances.extend(targetRegion.getVertices().begin(),
                                 targetRegion.getVertices().end(),
                                 targetFilter);
        }

        num sourceDifference = inf, targetDifference = inf;

        for(const Vertex& vertex : sourceRegion.getVertices())
        {
          if(!sourceDistances.explored(vertex))
          {
            continue;
          }

          sourceDifference = std::min(sourceDifference,
                                      sourceDistances.distance(vertex) - sourceBounds.distance(vertex));
        }

        for(const Vertex& vertex : targetRegion.getVertices())
        {
          if(!targetDistances.explored(vertex))
          {
            continue;
          }

          targetDifference = std::min(targetDifference,
                                      targetDistances.distance(vertex) - targetBounds.distance(vertex));
        }

        num delta = deviationSize*(value - bestValue) +
          boundaryDistances(source, target)[i] -
          boundaryDistances(source, target)[bestIndex];

        assert(delta >= 0);

        if(sourceDifference + targetDifference < -delta)
        {
          requiredValues.insert(value);
        }
      }

      Log(debug) << "Computed upper bounds for " << numValues << " values.";
    }
  }


  return requiredValues;
}

ValueSet
OuterValuePreprocessor::requiredValues(const Region& sourceRegion,
                                       const Region& targetRegion,
                                       const ValueSet& possibleValues) const
{
  DistanceMap boundaryDistances = findShortestPaths(sourceRegion, targetRegion);
  ValueSet valueSet(values.begin(), values.end());
  return requiredValues(sourceRegion, targetRegion, boundaryDistances, valueSet);
}

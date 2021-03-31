#include "simple_value_preprocessor.hh"

#include "log.hh"

#include "router/label_heap.hh"
#include "robust/reduced_costs.hh"

std::unordered_set<num>
SimpleValuePreprocessor::requiredValues(const Region& sourceRegion,
                                        const Region& targetRegion) const
{
  std::unordered_set<num> requiredValues;

  const std::unordered_set<Vertex>& sourceBoundaries =
    partition.boundaryVertices<Direction::OUTGOING>(sourceRegion);

  const std::unordered_set<Vertex>& targetBoundaries =
    partition.boundaryVertices<Direction::INCOMING>(targetRegion);

  DistanceMap boundaryDistances = findShortestPaths(sourceRegion, targetRegion);

  VertexPairMap<BoundValues> globalBounds;

  for(idx i = 0; i < values.size(); ++i)
  {
    const num& value = values[i];
    ReducedCosts reducedCosts(costs, deviations, value);

    std::unordered_map<Vertex, Vertex> associations;

    VertexPairMap<Bound> currentBounds;

    LabelHeap<RootedLabel> sourceHeap(graph);
    LabelHeap<RootedLabel> targetHeap(graph);

    auto sourceFilter = partition.regionFilter(sourceRegion);
    auto targetFilter = partition.regionFilter(targetRegion);

    Log(info) << "Computing lower bounds for path lengths";

    for(const Vertex& sourceBoundary : sourceBoundaries)
    {
      num minDistance = inf;
      Vertex minVertex;

      for(const Vertex& targetBoundary : targetBoundaries)
      {
        num nextDistance = boundaryDistances(sourceBoundary,
                                             targetBoundary)[i];

        if(nextDistance < minDistance)
        {
          minDistance = nextDistance;
          minVertex = targetBoundary;
        }

        Bound& currentBound = currentBounds(sourceBoundary,
                                            targetBoundary);

        currentBound.setMin(nextDistance + value*deviationSize);
        currentBound.setMax(nextDistance + value*deviationSize);
      }

      assert(minDistance < inf);

      associations[sourceBoundary] = minVertex;

      sourceHeap.update(RootedLabel(sourceBoundary,
                                    minDistance,
                                    sourceBoundary));

      Bound& currentBound = currentBounds(sourceBoundary,
                                          minVertex);

      currentBound.setMin(minDistance + value*deviationSize);
      currentBound.setMax(minDistance + value*deviationSize);
    }

    for(const Vertex& targetBoundary : targetBoundaries)
    {
      num minDistance = inf;
      Vertex minVertex;

      for(const Vertex& sourceBoundary : sourceBoundaries)
      {
        num nextDistance = boundaryDistances(sourceBoundary,
                                             targetBoundary)[i];

        if(nextDistance < minDistance)
        {
          minDistance = nextDistance;
          minVertex = sourceBoundary;
        }
      }

      assert(minDistance < inf);

      associations[targetBoundary] = minVertex;

      targetHeap.update(RootedLabel(targetBoundary,
                                    minDistance,
                                    targetBoundary));

      Bound& currentBound = currentBounds(minVertex,
                                          targetBoundary);

      currentBound.setMin(minDistance);
      currentBound.setMax(minDistance);
    }

    Log(info) << "Computing upper bounds for path lengths";

    while(!sourceHeap.isEmpty())
    {
      const RootedLabel& current = sourceHeap.extractMin();

      for(const Edge& edge : graph.getIncoming(current.getVertex()))
      {
        if(!sourceFilter(edge))
        {
          continue;
        }

        RootedLabel nextLabel(edge.getSource(),
                              current.getCost() + reducedCosts(edge),
                              current.getRoot());

        sourceHeap.update(nextLabel);
      }
    }

    while(!targetHeap.isEmpty())
    {
      const RootedLabel& current = targetHeap.extractMin();


      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        if(!targetFilter(edge))
        {
          continue;
        }

        RootedLabel nextLabel(edge.getTarget(),
                              current.getCost() + reducedCosts(edge),
                              current.getRoot());

        targetHeap.update(nextLabel);
      }
    }

    for(const Vertex& source : sourceRegion.getVertices())
    {
      const RootedLabel& label = sourceHeap.getLabel(source);

      assert(label.getCost() != inf);

      Vertex sourceBoundary = label.getRoot();
      Vertex targetBoundary = associations.at(sourceBoundary);

      Bound& currentBound = currentBounds(sourceBoundary,
                                          targetBoundary);

      currentBound.setMax(std::max(currentBound.getMax(),
                                   label.getCost() +
                                   boundaryDistances(sourceBoundary,
                                                     targetBoundary)[i] +
                                   value*deviationSize));
    }

    for(const Vertex& target : targetRegion.getVertices())
    {
      const RootedLabel& label = targetHeap.getLabel(target);

      assert(label.getCost() != inf);

      Vertex targetBoundary = label.getRoot();
      Vertex sourceBoundary = associations.at(targetBoundary);

      Bound& currentBound = currentBounds(sourceBoundary,
                                          targetBoundary);

      currentBound.setMax(std::max(currentBound.getMax(),
                                   label.getCost() +
                                   boundaryDistances(sourceBoundary,
                                                     targetBoundary)[i] +
                                   value*deviationSize));
    }

    Log(info) << "Updating global bounds";

    for(const Vertex& sourceBoundary : sourceBoundaries)
    {
      for(const Vertex& targetBoundary : targetBoundaries)
      {

        const Bound& currentBound = currentBounds(sourceBoundary,
                                                  targetBoundary);

        BoundValues& globalBound = globalBounds(sourceBoundary,
                                                targetBoundary);

        Log(info) << "Lower bound: "
                  << currentBound.getMin()
                  << ", upper bound: "
                  << currentBound.getMax();

        if(currentBound <= globalBound)
        {
          globalBound.setMin(currentBound.getMin());
          globalBound.setMax(currentBound.getMax());
          globalBound.getValues() = {value};
        }
        else if(globalBound <= currentBound)
        {
        }
        else
        {
          globalBound.setMin(std::min(globalBound.getMin(),
                                      currentBound.getMin()));

          globalBound.setMax(std::min(globalBound.getMax(),
                                      currentBound.getMax()));

          globalBound.getValues().push_back(value);
        }
      }
    }
  }

  for(const Vertex& sourceBoundary : sourceBoundaries)
  {
    for(const Vertex& targetBoundary : targetBoundaries)
    {
      const BoundValues& globalBound = globalBounds(sourceBoundary,
                                                    targetBoundary);

      std::cout << "[";

      for(const num& value : globalBound.getValues())
      {
        std::cout << value << ", ";
        requiredValues.insert(value);
      }

      std::cout << "]" << std::endl;
    }
  }

  return requiredValues;
}

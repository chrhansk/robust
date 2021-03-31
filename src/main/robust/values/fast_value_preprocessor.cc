#include "fast_value_preprocessor.hh"

#include "log.hh"

#include "router/label_heap.hh"
#include "router/router.hh"


std::unordered_set<num>
FastValuePreprocessor::requiredValues(const Region& sourceRegion,
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

    VertexPairMap<Bound> currentBounds;

    Log(info) << "Computing lower bounds for path lengths";

    setLowerBounds(sourceRegion, targetRegion,
                   sourceBoundaries, targetBoundaries,
                   boundaryDistances,
                   currentBounds,
                   i);

    // upper bound computation here

    Log(info) << "Computing upper bounds for path lengths";

    LabelHeap<RootedLabel> sourceHeap(graph);
    LabelHeap<RootedLabel> targetHeap(graph);

    for(const Vertex& sourceBoundary : sourceBoundaries)
    {
      num minDistance = inf;
      Vertex bestTarget;

      for(const Vertex& targetBoundary : targetBoundaries)
      {
        num nextDistance = boundaryDistances(sourceBoundary,
                                             targetBoundary)[i];

        if(nextDistance < minDistance)
        {
          minDistance = nextDistance;
          bestTarget = targetBoundary;
        }
      }

      sourceHeap.update(RootedLabel(sourceBoundary,
                                    minDistance,
                                    sourceBoundary));
    }

    for(const Vertex& targetBoundary : targetBoundaries)
    {
      num minDistance = inf;
      Vertex bestTarget;

      for(const Vertex& sourceBoundary : sourceBoundaries)
      {
        num nextDistance = boundaryDistances(sourceBoundary,
                                             targetBoundary)[i];

        if(nextDistance < minDistance)
        {
          minDistance = nextDistance;
          bestTarget = sourceBoundary;
        }
      }

      targetHeap.update(RootedLabel(targetBoundary,
                                    minDistance,
                                    targetBoundary));
    }

    auto sourceFilter = partition.regionFilter(sourceRegion);
    auto targetFilter = partition.regionFilter(targetRegion);

    Log(info) << "Solving shortest path problems";

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

    if(debuggingEnabled())
    {
      for(const Vertex& source : sourceRegion.getVertices())
      {
        const num cost = sourceHeap.getLabel(source).getCost();
        bool tight = false;

        for(const Vertex& target : targetRegion.getVertices())
        {
          Dijkstra dijkstra(graph);

          auto result = dijkstra.shortestPath(source, target, reducedCosts);

          if(!result.found)
          {
            continue;
          }

          assert(result.cost >= cost);

          if(result.cost == cost)
          {
            tight = true;
          }
        }

        assert(tight);
      }

      for(const Vertex& target : targetRegion.getVertices())
      {
        const num cost = targetHeap.getLabel(target).getCost();
        bool tight = false;

        for(const Vertex& source : sourceRegion.getVertices())
        {
          Dijkstra dijkstra(graph);

          auto result = dijkstra.shortestPath(source, target, reducedCosts);

          if(!result.found)
          {
            continue;
          }

          assert(result.cost >= cost);

          if(result.cost == cost)
          {
            tight = true;
          }
        }

        assert(tight);
      }
    }

    Log(info) << "Setting upper bounds";

    for(const Vertex& source : sourceRegion.getVertices())
    {
      const RootedLabel& sourceLabel = sourceHeap.getLabel(source);
      const Vertex& sourceBoundary = sourceLabel.getRoot();
      const RootedLabel& sourceRootLabel = sourceHeap.getLabel(sourceBoundary);

      for(const Vertex& target : targetRegion.getVertices())
      {
        const RootedLabel& targetLabel = targetHeap.getLabel(target);
        const Vertex& targetBoundary = targetLabel.getRoot();
        const RootedLabel& targetRootLabel = targetHeap.getLabel(targetBoundary);

        currentBounds(source, target)
          .setMax((sourceLabel.getCost() - sourceRootLabel.getCost()) +
                  (targetLabel.getCost() - targetRootLabel.getCost()) +
                  (boundaryDistances(sourceBoundary, targetBoundary)[i]) +
                  value*deviationSize);
      }
    }

    updateBounds(sourceRegion,
                 targetRegion,
                 globalBounds,
                 currentBounds,
                 value);

  }

  for(const Vertex& source : sourceRegion.getVertices())
  {
    for(const Vertex& target : targetRegion.getVertices())
    {
      const BoundValues& globalBound =
        globalBounds(source, target);

      for(const num& value : globalBound.getValues())
      {
        requiredValues.insert(value);
      }

      if(source == *(sourceRegion.getVertices().begin()) and
         target == *(targetRegion.getVertices().begin()))
      {
        Log(info) << "Lower bound: "
                  << globalBound.getMin()
                  << ", upper bound: "
                  << globalBound.getMax()
                  << ", #values : "
                  << globalBound.getValues().size();
      }
    }
  }

  return requiredValues;
}


void
FastValuePreprocessor::updateBounds(const Region& sourceRegion,
                                    const Region& targetRegion,
                                    VertexPairMap<BoundValues>& globalBounds,
                                    const VertexPairMap<Bound>& currentBounds,
                                    num value) const
{
  Log(info) << "Updating bounds";

  for(const Vertex& source : sourceRegion.getVertices())
  {
    for(const Vertex& target : targetRegion.getVertices())
    {
      BoundValues& globalBound = globalBounds(source, target);

      const Bound& currentBound = currentBounds(source, target);

      if(debuggingEnabled())
      {
        Dijkstra dijkstra(graph);

        ReducedCosts reducedCosts(costs, deviations, value);

        auto result = dijkstra.shortestPath(source, target, reducedCosts);

        num actualCost = result.cost + value*deviationSize;

        assert(currentBound.getMin() <= actualCost);
        assert(currentBound.getMax() >= actualCost);
      }

      if(currentBound <= globalBound)
      {
        globalBound.setMin(currentBound.getMin());
        globalBound.setMax(currentBound.getMax());
        globalBound.getValues() = ValueVector();
        globalBound.getValues().push_back(value);
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


void FastValuePreprocessor::setLowerBounds(const Region& sourceRegion,
                                           const Region& targetRegion,
                                           const std::unordered_set<Vertex>& sourceBoundaries,
                                           const std::unordered_set<Vertex>& targetBoundaries,
                                           const DistanceMap& boundaryDistances,
                                           VertexPairMap<Bound>& currentBounds,
                                           idx i) const
{
  const num& value = values[i];
  ReducedCosts reducedCosts(costs, deviations, value);

  LabelHeap<SimpleLabel> sourceHeap(graph);

  for(const Vertex& sourceBoundary : sourceBoundaries)
  {
    num minDist = inf;

    for(const Vertex& targetBoundary : targetBoundaries)
    {
      num nextDist =
        boundaryDistances(sourceBoundary,
                          targetBoundary)[i];

      if(nextDist < minDist)
      {
        minDist = nextDist;
      }
    }

    sourceHeap.update(SimpleLabel(sourceBoundary, minDist));
  }

  auto sourceFilter = partition.regionFilter(sourceRegion);

  while(!sourceHeap.isEmpty())
  {
    const SimpleLabel& current = sourceHeap.extractMin();

    for(const Edge& edge : graph.getIncoming(current.getVertex()))
    {
      if(!sourceFilter(edge))
      {
        continue;
      }

      SimpleLabel nextLabel(edge.getSource(),
                            current.getCost() + reducedCosts(edge));

      sourceHeap.update(nextLabel);
    }
  }

  LabelHeap<Label> targetHeap(graph);

  for(const Vertex& targetBoundary : targetBoundaries)
  {
    targetHeap.update(Label(targetBoundary, Edge(), 0));
  }

  auto targetFilter = partition.regionFilter(targetRegion);

  for(const Vertex& target : targetRegion.getVertices())
  {
    while(!targetHeap.isEmpty())
    {
      if(targetHeap.getLabel(target).getState() == State::SETTLED)
      {
        break;
      }

      const Label& current = targetHeap.extractMin();

      for(const Edge& edge : graph.getOutgoing(current.getVertex()))
      {
        // TODO: use region filter here?

        if(!targetFilter(edge))
        {
          continue;
        }

        Label nextLabel = Label(edge.getTarget(),
                                edge, current.getCost() + reducedCosts(edge));

        targetHeap.update(nextLabel);
      }
    }
  }

  // fill in the lower bounds
  for(const Vertex& source : sourceRegion.getVertices())
  {
    for(const Vertex& target : targetRegion.getVertices())
    {
      const num lowerBound = sourceHeap.getLabel(source).getCost() +
        targetHeap.getLabel(target).getCost();

      currentBounds(source, target).setMin(lowerBound + value*deviationSize);
    }
  }
}

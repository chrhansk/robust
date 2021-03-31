#include "bucket_value_preprocessor.hh"

#include "log.hh"

#include "router/label.hh"
#include "router/label_heap.hh"
#include "router/distance_tree.hh"

#include "robust/reduced_costs.hh"

std::pair<std::vector<class BucketValuePreprocessor::Bucket>,
          std::vector<class BucketValuePreprocessor::Bucket>>
BucketValuePreprocessor::medianPartition(const Boundary& sourceBoundary,
                                         const Boundary& targetBoundary) const
{
  std::vector<Bucket> sourceBuckets(numBuckets), targetBuckets(numBuckets);

  auto sourceFilter = partition.regionFilter(sourceBoundary.getRegion());
  auto targetFilter = partition.regionFilter(targetBoundary.getRegion());

  const idx i = 0;
  const num& value = values[i];
  ReducedCosts reducedCosts(costs, deviations, value);

  auto boundaryDistances = findDistances(sourceBoundary.getVertices(),
                                         targetBoundary.getVertices(),
                                         reducedCosts);

  DistanceTree<Direction::INCOMING> sourceTree(graph, reducedCosts);
  DistanceTree<Direction::OUTGOING> targetTree(graph, reducedCosts);

  for(const Vertex& source : sourceBoundary.getVertices())
  {
    num minDistance = inf;

    for(const Vertex& target : targetBoundary.getVertices())
    {
      num nextDistance = boundaryDistances(source,
                                           target);

      if(nextDistance < minDistance)
      {
        minDistance = nextDistance;
      }
    }

    sourceTree.add(source, minDistance);
  }

  for(const Vertex& target : targetBoundary.getVertices())
  {
    num minDistance = inf;

    for(const Vertex& source : sourceBoundary.getVertices())
    {
      num nextDistance = boundaryDistances(source,
                                           target);

      if(nextDistance < minDistance)
      {
        minDistance = nextDistance;
      }
    }

    targetTree.add(target, minDistance);
  }

  idx bucketSize = (idx)
    std::ceil(sourceBoundary.getRegion().getVertices().size() /
              ((float) numBuckets));

  idx current = 0;

  while(!sourceTree.done())
  {
    Vertex vertex = sourceTree.next(sourceFilter);

    Bucket& bucket = sourceBuckets[current];

    if(bucket.getVertices().size() < bucketSize)
    {
      bucket.getVertices().push_back(vertex);
    }
    else
    {
      sourceBuckets[++current].getVertices().push_back(vertex);
    }
  }

  bucketSize = (idx)
    std::ceil(targetBoundary.getRegion().getVertices().size() /
              ((float) numBuckets));

  current = 0;

  while(!targetTree.done())
  {
    Vertex vertex = targetTree.next(targetFilter);

    Bucket& bucket = targetBuckets[current];

    if(bucket.getVertices().size() < bucketSize)
    {
      bucket.getVertices().push_back(vertex);
    }
    else
    {
      targetBuckets[++current].getVertices().push_back(vertex);
    }
  }

  while(sourceBuckets.rbegin()->getVertices().empty())
  {
    sourceBuckets.pop_back();
  }

  while(targetBuckets.rbegin()->getVertices().empty())
  {
    targetBuckets.pop_back();
  }

  if(debuggingEnabled())
  {
    for(const Bucket& bucket : sourceBuckets)
    {
      assert(!bucket.getVertices().empty());
    }

    for(const Bucket& bucket : targetBuckets)
    {
      assert(!bucket.getVertices().empty());
    }
  }

  return std::make_pair(sourceBuckets, targetBuckets);
}

std::pair<std::vector<BucketValuePreprocessor::Bucket>,
          std::vector<BucketValuePreprocessor::Bucket>>
BucketValuePreprocessor::meanPartition(const Boundary& sourceBoundary,
                                       const Boundary& targetBoundary) const
{
  std::vector<Bucket> sourceBuckets, targetBuckets;

  auto sourceFilter = partition.regionFilter(sourceBoundary.getRegion());
  auto targetFilter = partition.regionFilter(targetBoundary.getRegion());

  const idx i = 0;
  const num& value = values[i];
  ReducedCosts reducedCosts(costs, deviations, value);

  auto boundaryDistances = findDistances(sourceBoundary.getVertices(),
                                         targetBoundary.getVertices(),
                                         reducedCosts);

  num minSourceValue = inf, minTargetValue = inf;
  num maxSourceValue = inf, maxTargetValue = inf;

  DistanceTree<Direction::OUTGOING> sourceTree(graph, reducedCosts);
  DistanceTree<Direction::OUTGOING> targetTree(graph, reducedCosts);

  for(const Vertex& source : sourceBoundary.getVertices())
  {
    num minDistance = inf;

    for(const Vertex& target : targetBoundary.getVertices())
    {
      num nextDistance = boundaryDistances(source,
                                           target);

      if(nextDistance < minDistance)
      {
        minDistance = nextDistance;
      }
    }

    minSourceValue = std::min(minSourceValue, minDistance);

    sourceTree.add(source, minDistance);
  }

  for(const Vertex& target : targetBoundary.getVertices())
  {
    num minDistance = inf;

    for(const Vertex& source : sourceBoundary.getVertices())
    {
      num nextDistance = boundaryDistances(source,
                                           target);

      if(nextDistance < minDistance)
      {
        minDistance = nextDistance;
      }
    }

    minTargetValue = std::min(minTargetValue, minDistance);

    targetTree.add(target, minDistance);
  }

  sourceTree.extend(sourceFilter);
  targetTree.extend(targetFilter);

  for(idx i = 0; i < numBuckets; ++i)
  {
    sourceBuckets.push_back(Bucket());
    targetBuckets.push_back(Bucket());
  }

  maxSourceValue = sourceTree.maxDistance();
  maxTargetValue = targetTree.maxDistance();

  for(const Vertex& source : sourceBoundary.getRegion().getVertices())
  {
    if(!sourceTree.explored(source))
    {
      assert(maxSourceValue == inf);
      continue;
    }

    num distance = sourceTree.distance(source);
    assert(distance >= minSourceValue and distance <= maxSourceValue);

    idx bucket = (idx) floor((distance - minSourceValue) /
                             ((float) maxSourceValue - minSourceValue)
                             * (numBuckets - 1));

    sourceBuckets[bucket].getVertices().push_back(source);
  }

  for(const Vertex& target : targetBoundary.getRegion().getVertices())
  {
    if(!targetTree.explored(target))
    {
      assert(maxTargetValue == inf);
      continue;
    }

    num distance = targetTree.distance(target);
    assert(distance >= minTargetValue and distance <= maxTargetValue);

    idx bucket = (idx) floor((distance - minTargetValue) /
                             ((float) maxTargetValue - minTargetValue)
                             * (numBuckets - 1));

    targetBuckets[bucket].getVertices().push_back(target);
  }

  std::vector<Bucket> sources, targets;

  for(const Bucket& bucket : sourceBuckets)
  {
    if(!bucket.getVertices().empty())
    {
      sources.push_back(bucket);
    }
  }

  for(const Bucket& bucket : targetBuckets)
  {
    if(!bucket.getVertices().empty())
    {
      targets.push_back(bucket);
    }
  }

  return std::make_pair(sources, targets);
}

ValueSet
BucketValuePreprocessor::requiredValues(const Region& sourceRegion,
                                        const Region& targetRegion,
                                        const ValueSet& possibleValues) const
{
  std::unordered_set<num> requiredValues;

  Boundary sourceBoundary = partition.getBoundary<Direction::OUTGOING>(sourceRegion);
  Boundary targetBoundary = partition.getBoundary<Direction::INCOMING>(targetRegion);

  std::vector<Bucket> sourceBuckets, targetBuckets;

  std::tie(sourceBuckets, targetBuckets) = medianPartition(sourceBoundary,
                                                           targetBoundary);

  PairMap<idx, std::vector<Bound>> globalBounds;

  for(idx j = 0; j < sourceBuckets.size(); ++j)
  {
    for(idx k = 0; k < targetBuckets.size(); ++k)
    {
      globalBounds.put(j, k, {});
    }
  }

  auto sourceFilter = partition.regionFilter(sourceRegion);
  auto targetFilter = partition.regionFilter(targetRegion);

  for(const num& value : possibleValues)
  {
    Log(info) << "Computing boundary distances for " << value;

    ReducedCosts reducedCosts(costs, deviations, value);

    auto boundaryDistances = findDistances(sourceBoundary.getVertices(),
                                           targetRegion.getVertices(),
                                           reducedCosts);

    Log(info) << "Computing bounds for " << value;

    PairMap<idx, Bound> currentBounds;

    for(idx j = 0; j < sourceBuckets.size(); ++j)
    {
      for(idx k = 0; k < targetBuckets.size(); ++k)
      {
        currentBounds.put(j, k, Bound());
      }
    }

    // Compute lower bounds
    {
      for(idx j = 0; j < sourceBuckets.size();++j)
      {
        Bucket& sourceBucket = sourceBuckets[j];

        DistanceTree<Direction::OUTGOING> sourceTree(graph, reducedCosts);

        for(const Vertex& vertex : sourceBucket.getVertices())
        {
          sourceTree.add(vertex, 0);
        }

        sourceTree.extend(sourceFilter);

        if(debuggingEnabled()) {

          bool found = false;

          for(const Vertex& vertex : sourceBoundary.getVertices())
          {
            if(sourceTree.explored(vertex))
            {
              found = true;
              break;
            }
          }

          assert(found);
        }

        DistanceTree<Direction::OUTGOING> targetTree(graph, reducedCosts);

        for(const Vertex& target : targetBoundary.getVertices())
        {
          num minDistance = inf;

          for(const Vertex& source : sourceBoundary.getVertices())
          {
            if(sourceTree.explored(source))
            {
              const num nextDistance = sourceTree.distance(source) +
                boundaryDistances(source, target);

              minDistance = std::min(nextDistance, minDistance);
            }
          }

          if(minDistance != inf)
          {
            targetTree.add(target, minDistance);
          }
        }

        targetTree.extend(targetFilter);

        for(idx k = 0; k < targetBuckets.size(); ++k)
        {
          const Bucket& targetBucket = targetBuckets[k];

          num minDistance = inf;

          for(const Vertex& target : targetBucket.getVertices())
          {
            assert(targetTree.explored(target));

            const num currentDistance = targetTree.distance(target);

            minDistance = std::min(currentDistance, minDistance);
          }

          assert(minDistance != inf);

          currentBounds(j, k).setMin(minDistance + value*deviationSize);
        }
      }
    }

    // Compute upper bounds
    {
      LabelHeap<RootedLabel> heap(graph);

      for(const Vertex& source : sourceBoundary.getVertices())
      {
        num minDistance = inf;

        for(const Vertex& target : targetBoundary.getVertices())
        {
          minDistance = std::min(minDistance, boundaryDistances(source, target));
        }

        heap.update(RootedLabel(source, minDistance, source));
      }

      while(!heap.isEmpty())
      {
        const RootedLabel& current = heap.extractMin();

        for(const Edge& edge : graph.getIncoming(current.getVertex()))
        {
          if(!sourceFilter(edge))
          {
            continue;
          }

          heap.update(RootedLabel(edge.getSource(),
                                  current.getCost() + reducedCosts(edge),
                                  current.getRoot()));
        }
      }

      for(idx j = 0; j < sourceBuckets.size(); ++j)
      {
        const Bucket& sourceBucket = sourceBuckets[j];

        VertexMap<num> rootBounds(graph, -1);

        for(const Vertex& source : sourceBucket.getVertices())
        {
          const RootedLabel& sourceLabel = heap.getLabel(source);

          assert(sourceLabel.getState() == State::SETTLED);

          const RootedLabel& rootLabel = heap.getLabel(sourceLabel.getRoot());
          const Vertex& root = rootLabel.getVertex();

          assert(sourceLabel.getCost() >= rootLabel.getCost());

          rootBounds(root) = std::max(rootBounds(root), sourceLabel.getCost() - rootLabel.getCost());
        }

        for(idx k = 0; k < targetBuckets.size(); ++k)
        {
          const Bucket& targetBucket = targetBuckets[k];

          num maxDistance = -1;

          bool rootFound = false;

          for(const Vertex& source : sourceBoundary.getVertices())
          {
            if(rootBounds(source) == -1)
            {
              continue;
            }

            rootFound = true;

            num currentDistance = -1;

            for(const Vertex& target : targetBucket.getVertices())
            {
              num boundaryDistance = boundaryDistances(source, target);

              currentDistance = std::max(currentDistance,
                                         rootBounds(source) + boundaryDistance);
            }

            maxDistance = std::max(currentDistance, maxDistance);
          }

          assert(rootFound);

          currentBounds(j, k).setMax(maxDistance + value*deviationSize);
        }
      }
    }

    // Check consistency
    if(debuggingEnabled())
    {
      for(idx j = 0; j < sourceBuckets.size(); ++j)
      {
        const Bucket& sourceBucket = sourceBuckets[j];

        for(const Vertex& source : sourceBucket.getVertices())
        {
          for(idx k = 0; k < targetBuckets.size(); ++k)
          {
            const Bucket& targetBucket = targetBuckets[k];

            Bound& bound = currentBounds(j, k);

            num minCost = inf, maxCost = -1;

            for(const Vertex& target : targetBucket.getVertices())
            {
              Dijkstra dijkstra(graph);

              SearchResult result = dijkstra.shortestPath(source, target, reducedCosts);

              assert(result.found);

              num cost = result.path.cost(reducedCosts) + value*deviationSize;

              minCost = std::min(minCost, cost);
              maxCost = std::max(maxCost, cost);
            }

            assert(minCost >= bound.getMin());
            assert(maxCost <= bound.getMax());

            /*
            std::cout << "Bucket ("
                      << j
                      << ", "
                      << k
                      << "): lower = "
                      << bound.getMin()
                      << ", best = "
                      << minCost
                      << ", worst = "
                      << maxCost
                      << ", upper = "
                      << bound.getMax()
                      << std::endl;
            */
          }
        }
      }
    }

    // Update global bounds
    for(idx j = 0; j < sourceBuckets.size(); ++j)
    {
      for(idx k = 0; k < targetBuckets.size(); ++k)
      {
        globalBounds(j, k).push_back(currentBounds(j, k));
      }
    }
  }

  for(idx j = 0; j < sourceBuckets.size(); ++j)
  {
    for(idx k = 0; k < targetBuckets.size(); ++k)
    {
      num maxValue = inf;

      float minGap = inf, maxGap = 0;

      for(const Bound& bound : globalBounds(j, k))
      {
        maxValue = std::min(maxValue, bound.getMax());

        float gap = 100*(bound.getMax() - bound.getMin()) / (float) bound.getMin();

        minGap = std::min(minGap, gap);
        maxGap = std::max(maxGap, gap);
      }

      idx count = 0;

      for(idx i = 0; i < values.size(); ++i)
      {
        const num& value = values[i];
        if(globalBounds(j, k)[i].getMin() <= maxValue)
        {
          requiredValues.insert(value);
          ++count;
        }
      }

      Log(info) << "Min gap: " << minGap << ", max gap: " << maxGap
                << " for (" << j << ", " << k << ")"
                << ", #values: " << count;
    }
  }

  return requiredValues;
}

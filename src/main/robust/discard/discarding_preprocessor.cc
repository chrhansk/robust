#include "discarding_preprocessor.hh"

#include "log.hh"

#include "robust/arcflags/robust_arcflag_preprocessor.hh"
#include "router/distance_tree.hh"

DiscardingPreprocessor::DiscardingPreprocessor(const Graph& graph,
                                               const EdgeFunc<num>& costs,
                                               const EdgeFunc<num>& deviations,
                                               const Partition& partition)
  : graph(graph),
    costs(costs),
    deviations(deviations),
    partition(partition),
    arcFlags(graph, partition)
{
  for(const Edge& edge : graph.getEdges())
  {
    num deviation = deviations(edge);

    auto it = valueEdges.find(deviation);

    if(it == valueEdges.end())
    {
      valueEdges.insert(std::make_pair(deviation, std::vector<Edge>{edge}));
    }
    else
    {
      it->second.push_back(edge);
    }
  }

  Log(info) << "Computing distance bounds between regions of a partition of size "
            << partition.getRegions().size();

  for(const Region& sourceRegion : partition.getRegions())
  {
    auto sourceBoundary = partition.boundaryVertices<Direction::OUTGOING>(sourceRegion);

    for(const Region& targetRegion : partition.getRegions())
    {
      distances.put(sourceRegion, targetRegion, inf);
    }

    for(const Vertex& source : sourceBoundary)
    {
      DistanceTree<Direction::OUTGOING> distanceTree(graph, costs, source);

      for(const Region& targetRegion : partition.getRegions())
      {
        if(sourceRegion == targetRegion)
        {
          continue;
        }

        auto targetBoundary = partition.boundaryVertices<Direction::INCOMING>(targetRegion);

        distanceTree.extend(targetBoundary.begin(), targetBoundary.end());

        for(const Vertex& target : targetBoundary)
        {
          assert(distanceTree.explored(target));

          distances(sourceRegion, targetRegion) = std::min(distances(sourceRegion, targetRegion),
                                                           distanceTree.distance(target));
        }
      }
    }
  }

  {
    ValueVector valueCandidates;
    valueCandidates.reserve(valueEdges.size());

    for(auto pair : valueEdges)
    {
      valueCandidates.push_back(pair.first);
    }

    std::sort(valueCandidates.begin(), valueCandidates.end(),
              [&](const num& first, const num& second) -> bool
              {
                auto fit = valueEdges.find(first);
                auto sit = valueEdges.find(second);

                assert(fit != valueEdges.end());
                assert(sit != valueEdges.end());

                return fit->second.size() > sit->second.size();
              });

    valueCandidates.resize(10);

    std::sort(valueCandidates.begin(), valueCandidates.end(),
              std::greater<num>{});

    bestValues = ValueSet(valueCandidates.begin(), valueCandidates.end());

    RobustArcFlagPreprocessor preprocessor(graph, costs, deviations,
                                           partition, valueCandidates);

    preprocessor.computeFlags(arcFlags, true);
  }

  auto it = valueEdges.begin();
  while(it != valueEdges.end())
  {
    if(it->second.size() >= 100)
    {
      it = valueEdges.erase(it);
    }
    else
    {
      ++it;
    }
  }

  Log(info) << "Collected " << valueEdges.size() << " values ";

  Log(info) << "Computed distance bounds";
}


bool DiscardingPreprocessor::canDiscard(Vertex source,
                                        Vertex target,
                                        num value,
                                        idx deviationSize,
                                        num upperBound) const
{
  auto it = valueEdges.find(value);

  if(it == valueEdges.end())
  {
    return false;
  }

  const Region& sourceRegion = partition.getRegion(source);
  const Region& targetRegion = partition.getRegion(target);

  if(sourceRegion == targetRegion)
  {
    return false;
  }

  for(const Edge& edge : it->second)
  {
    assert(deviations(edge) == value);

    const Region& firstRegion = partition.getRegion(edge.getSource());
    const Region& lastRegion = partition.getRegion(edge.getSource());

    const num lowerBound = deviationSize*value +
      distances(sourceRegion, firstRegion) +
      distances(lastRegion, targetRegion) +
      costs(edge);

    if(lowerBound < upperBound)
    {
      return false;
    }
  }

  return true;
}

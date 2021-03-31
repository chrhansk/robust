#include "abstract_value_preprocessor.hh"

#include "log.hh"

#include "router/distance_tree.hh"
#include "router/label.hh"
#include "router/label_heap.hh"

#include "robust/reduced_costs.hh"

DistanceMap AbstractValuePreprocessor::findShortestPaths(const Region& sourceRegion,
                                                         const Region& targetRegion) const
{
  const auto& sourceBoundaries = partition.boundaryVertices<Direction::OUTGOING>(sourceRegion);
  const auto& targetBoundaries = partition.boundaryVertices<Direction::INCOMING>(targetRegion);

  DistanceMap boundaryDistances;

  Log(info) << "Computing shortest paths between regions";

  Log(info) << "Source region boundary size: " << sourceBoundaries.size();
  Log(info) << "Target region boundary size: " << targetBoundaries.size();

  idx i = 0;

  for(const Vertex& source : sourceBoundaries)
  {
    Log(info) << "Computing "
              << values.size()
              << " shortest path trees ["
              << ++i << "/"
              << sourceBoundaries.size()
              << "]";

    for(const Vertex& target : targetBoundaries)
    {
      boundaryDistances.put(source, target, ValueVector());
    }

    for(const num& value : values)
    {
      ReducedCosts reducedCosts(costs, deviations, value);

      LabelHeap<Label> heap(graph);

      heap.update(Label(source, Edge(), 0));

      for(const Vertex& target : targetBoundaries)
      {
        while(!heap.isEmpty())
        {
          if(heap.getLabel(target).getState() == State::SETTLED)
          {
            break;
          }

          const Label& current = heap.extractMin();

          for(const Edge& edge : graph.getOutgoing(current.getVertex()))
          {
            Label nextLabel = Label(edge.getTarget(),
                                    edge, current.getCost() + reducedCosts(edge));

            heap.update(nextLabel);
          }
        }

        assert(heap.getLabel(target).getState() == State::SETTLED);

        boundaryDistances(source, target).push_back(heap.getLabel(target).getCost());
      }
    }
  }

  return boundaryDistances;
}

VertexPairMap<num>
AbstractValuePreprocessor::findDistances(const std::vector<Vertex>& sources,
                                         const std::vector<Vertex>& targets,
                                         const EdgeFunc<num>& costFunc) const
{
  VertexPairMap<num> boundaryDistances;

  for(const Vertex& source : sources)
  {
    DistanceTree<Direction::OUTGOING> distanceTree(graph, costFunc);

    distanceTree.add(source, 0);

    distanceTree.extend(targets.begin(),
                        targets.end());

    for(const Vertex& target : targets)
    {
      num distance = inf;

      if(distanceTree.explored(target))
      {
        distance = distanceTree.distance(target);
      }

      boundaryDistances.put(source, target, distance);
    }
  }

  return boundaryDistances;
}


ValueSet
AbstractValuePreprocessor::requiredValues(const Region& sourceRegion,
                                          const Region& targetRegion) const
{
  ValueSet valueSet(values.begin(), values.end());
  return requiredValues(sourceRegion, targetRegion, valueSet);
}

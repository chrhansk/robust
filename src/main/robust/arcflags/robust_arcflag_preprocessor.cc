#include "robust_arcflag_preprocessor.hh"

#include <unordered_set>

#include <tbb/tbb.h>

#include "log.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"

RobustArcFlagPreprocessor::RobustArcFlagPreprocessor(const Graph& graph,
                                                     const EdgeFunc<num>& costs,
                                                     const EdgeFunc<num>& deviations,
                                                     const Partition& partition)
  : AbstractArcFlagPreprocessor(graph, costs, deviations, partition)
{
}

RobustArcFlagPreprocessor::RobustArcFlagPreprocessor(const Graph& graph,
                                                     const EdgeFunc<num>& costs,
                                                     const EdgeFunc<num>& deviations,
                                                     const Partition& partition,
                                                     const ValueVector& values)
  : AbstractArcFlagPreprocessor(graph, costs, deviations, partition, values)
{
}

template<Direction direction>
void RobustArcFlagPreprocessor::computeFlags(const Vertex& vertex,
                                             RobustArcFlags& flags) const
{
  const Region& region = partition.getRegion(vertex);

  for(const num& value : values)
  {
    ReducedCosts reducedCosts(costs, deviations, value);

    LabelHeap<Label> heap(graph);
    heap.update(Label(vertex, Edge(), 0));

    while(!heap.isEmpty())
    {
      const Label& current = heap.extractMin();
      Vertex currentVertex = current.getVertex();

      if(currentVertex != vertex)
      {
        flags.extend(current.getEdge(), region, value);

        assert(flags.filter(current.getEdge(), region, value));

        if(region == partition.getRegion(currentVertex))
        {
          continue;
        }
      }

      for(const Edge& edge : graph.getEdges(currentVertex, direction))
      {
        Vertex nextVertex = edge.getEndpoint(direction);

        Label nextLabel = Label(nextVertex,
                                edge,
                                current.getCost() + reducedCosts(edge));

        heap.update(nextLabel);
      }
    }
  }
}

template <Direction direction>
void RobustArcFlagPreprocessor::computeFlagsParallel(const std::unordered_set<Vertex>& vertices,
                                                     RobustArcFlags& flags) const
{
  tbb::spin_mutex mutex;

  tbb::parallel_do(vertices.begin(),
                   vertices.end(),
                   [this, &mutex, &flags](const Vertex& vertex)
                   {
                     EdgeMap<num> lowerBound(graph, inf),
                       upperBound(graph, 0);

                     const Region& vertexRegion = partition.getRegion(vertex);

                     for(auto it = values.rbegin(); it != values.rend(); ++it)
                     {
                       num value = *it;
                       ReducedCosts reducedCosts(costs, deviations, value);

                       LabelHeap<Label> heap(graph);
                       heap.update(Label(vertex, Edge(), 0));

                       while(!heap.isEmpty())
                       {
                         const Label& current = heap.extractMin();
                         Vertex currentVertex = current.getVertex();

                         if(current.getVertex() != vertex)
                         {
                           const Edge& edge = current.getEdge();

                           lowerBound(edge) = std::min(lowerBound(edge),
                                                       value);

                           upperBound(edge) = std::max(upperBound(edge),
                                                       value);

                           if(vertexRegion ==
                              partition.getRegion(current.getVertex()))
                           {
                             continue;
                           }
                         }

                         for(const Edge& edge : graph.getEdges(currentVertex,
                                                               direction))
                         {
                           Vertex nextVertex = edge.getEndpoint(direction);
                           const num nextCost = current.getCost() +
                             reducedCosts(edge);

                           Label nextLabel = Label(nextVertex, edge, nextCost);

                           heap.update(nextLabel);
                         }
                       }
                     }

                     {
                       tbb::spin_mutex::scoped_lock lock(mutex);

                       for(const Edge& edge : graph.getEdges())
                       {
                         num lower = lowerBound(edge);
                         num upper = upperBound(edge);

                         if(lower <= upper)
                         {
                           flags.extend(edge, vertexRegion, lower);
                           flags.extend(edge, vertexRegion, upper);
                         }
                       }
                     }
                   });
}

void RobustArcFlagPreprocessor::computeFlags(RobustArcFlags& incomingFlags,
                                             RobustArcFlags& outgoingFlags,
                                             bool parallelComputation) const
{
  std::vector<Edge> overlappingEdges;

  Log(info) << "Computing arc flags for " << values.size()
            << " values [parallel = "
            << std::boolalpha
            << parallelComputation
            << "]";

  for(const Edge& edge : graph.getEdges())
  {
    const Region& sourceRegion = partition.getRegion(edge.getSource());
    const Region& targetRegion = partition.getRegion(edge.getTarget());

    if(sourceRegion == targetRegion)
    {
      incomingFlags.extend(edge,
                           targetRegion);

      outgoingFlags.extend(edge,
                           sourceRegion);

      if(debuggingEnabled())
      {
        for(const num& value : values)
        {
          assert(incomingFlags.filter(edge, targetRegion, value));
          assert(outgoingFlags.filter(edge, sourceRegion, value));
        }
      }
    }
    else
    {
      overlappingEdges.push_back(edge);
    }
  }

  Log(info) << "Found " << overlappingEdges.size()
            << " overlapping edges";

  std::unordered_set<Vertex> sourceVertices, targetVertices;

  for(const Edge& edge : overlappingEdges)
  {
    sourceVertices.insert(edge.getSource());
    targetVertices.insert(edge.getTarget());
  }

  if(parallelComputation)
  {
    computeFlagsParallel<Direction::OUTGOING>(sourceVertices,
                                              outgoingFlags);

    computeFlagsParallel<Direction::INCOMING>(targetVertices,
                                              incomingFlags);
  }
  else
  {
    for(const Vertex& vertex : sourceVertices)
    {
      computeFlags<Direction::OUTGOING>(vertex, outgoingFlags);
    }

    for(const Vertex& vertex : targetVertices)
    {
      computeFlags<Direction::INCOMING>(vertex, incomingFlags);
    }
  }
}

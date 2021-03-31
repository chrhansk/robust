#ifndef VALUE_ARCFLAG_PREPROCESSOR_HH
#define VALUE_ARCFLAG_PREPROCESSOR_HH

#include <unordered_map>

#include <tbb/tbb.h>

#include "log.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "arcflags/region_pair_map.hh"

#include "robust/reduced_costs.hh"

#include "abstract_arcflag_preprocessor.hh"

class ValueArcFlagPreprocessor : public AbstractArcFlagPreprocessor
{
private:
  template <Direction direction>
  void
  computeFlagsParallel(const Region& region,
                       const RegionPairMap<ValueVector>& possibleValues,
                       RobustArcFlags& flags,
                       tbb::spin_mutex& mutex) const;

public:
  ValueArcFlagPreprocessor(const Graph& graph,
                           const EdgeFunc<num>& costs,
                           const EdgeFunc<num>& deviations,
                           const Partition& partition);

  template <Direction direction>
  void computeFlags(const Region& region,
                    const RegionPairMap<ValueVector>& possibleValues,
                    RobustArcFlags& flags) const;

  void computeFlags(RobustArcFlags& incomingFlags,
                    RobustArcFlags& outgoingFlags,
                    const RegionPairMap<ValueVector>& possibleValues,
                    bool parallelComputation) const;

  template<class Flags>
  void computeFlags(Bidirected<Flags>& flags,
                    const RegionPairMap<ValueVector>& possibleValues,
                    bool parallelComputation) const;
};

template<class Flags>
void
ValueArcFlagPreprocessor::computeFlags(Bidirected<Flags>& flags,
                                       const RegionPairMap<ValueVector>& possibleValues,
                                       bool parallelComputation) const
{
  computeFlags(flags.get(Direction::INCOMING),
               flags.get(Direction::OUTGOING),
               possibleValues,
               parallelComputation);
}

template <Direction direction>
void
ValueArcFlagPreprocessor::computeFlags(const Region& region,
                                       const RegionPairMap<ValueVector>& possibleValues,
                                       RobustArcFlags& flags) const
{
  std::unordered_map<num, std::vector<const Region*>> valueRegions;

  Log(info) << "Computing flags for region";

  for(const Vertex& vertex : region.getVertices())
  {
    for(const Edge& edge : graph.getAdjacentEdges(vertex))
    {
      flags.extend(edge, region);
    }
  }

  for(const Region& otherRegion : partition.getRegions())
  {
    if(region == otherRegion)
    {
      continue;
    }

    const ValueVector& regionValues = (direction == Direction::OUTGOING) ?
      possibleValues(region, otherRegion) :
      possibleValues(otherRegion, region);

    for(const num& value : regionValues)
    {
      valueRegions[value].push_back(&otherRegion);

      for(const Vertex& vertex : otherRegion.getVertices())
      {
        for(const Edge& edge : graph.getAdjacentEdges(vertex))
        {
          flags.extend(edge, region, value);
        }
      }
    }
  }

  auto boundary = partition.getBoundary<direction>(region);

  for(auto it : valueRegions)
  {
    num value = it.first;
    ReducedCosts reducedCosts(costs, deviations, value);

    Log(debug) << "Settling "
               << it.second.size()
               << " regions with respect to "
               << boundary.getVertices().size()
               << " boundary vertices";

    for(const Vertex& source : boundary.getVertices())
    {
      LabelHeap<Label> heap(graph);
      heap.update(Label(source, Edge(), 0));

      // explore all required regions
      for(auto rit : it.second)
      {
        const Region& otherRegion = *rit;
        for(const Vertex& target : otherRegion.getVertices())
        {
          while(!heap.isEmpty())
          {
            if(heap.getLabel(target).getState() == State::SETTLED)
            {
              break;
            }

            const Label& current = heap.extractMin();
            Vertex currentVertex = current.getVertex();

            for(const Edge& edge : graph.getEdges(currentVertex, direction))
            {
              Vertex nextVertex = edge.getEndpoint(direction);
              const num nextCost = current.getCost() + reducedCosts(edge);

              Label nextLabel = Label(nextVertex, edge, nextCost);

              heap.update(nextLabel);
            }
          }
        }
      }

      VertexSet considered(graph);
      considered.insert(source);

      for(auto rit : it.second)
      {
        const Region& otherRegion = *rit;

        for(const Vertex& target : otherRegion.getVertices())
        {
          Label current = heap.getLabel(target);

          assert(current.getState() == State::SETTLED);

          while(!(considered.contains(current.getVertex())))
          {
            Edge edge = current.getEdge();
            flags.extend(edge, region, value);
            considered.insert(current.getVertex());
            current = heap.getLabel(edge.getEndpoint(opposite(direction)));
          }
        }
      }
    }
  }
}

template <Direction direction>
void
ValueArcFlagPreprocessor::computeFlagsParallel(const Region& region,
                                               const RegionPairMap<ValueVector>& possibleValues,
                                               RobustArcFlags& flags,
                                               tbb::spin_mutex& mutex) const
{
  std::unordered_map<num, std::vector<const Region*>> valueRegions;

  Log(info) << "Computing flags for region";

  for(const Region& otherRegion : partition.getRegions())
  {
    if(region == otherRegion)
    {
      continue;
    }

    const ValueVector& values = direction == Direction::OUTGOING ?
      possibleValues(region, otherRegion) :
      possibleValues(otherRegion, region);

    for(const num& value : values)
    {
      valueRegions[value].push_back(&otherRegion);
    }
  }

  auto boundary = partition.getBoundary<direction>(region);

  for(auto it : valueRegions)
  {
    std::vector<Edge> flaggedEdges;

    num value = it.first;
    ReducedCosts reducedCosts(costs, deviations, value);

    Log(debug) << "Settling "
               << it.second.size()
               << " regions with respect to "
               << boundary.getVertices().size()
               << " boundary vertices";

    for(const Vertex& source : boundary.getVertices())
    {
      LabelHeap<Label> heap(graph);
      heap.update(Label(source, Edge(), 0));

      // explore all required regions
      for(auto rit : it.second)
      {
        const Region& otherRegion = *rit;
        for(const Vertex& target : otherRegion.getVertices())
        {
          while(!heap.isEmpty())
          {
            if(heap.getLabel(target).getState() == State::SETTLED)
            {
              break;
            }

            const Label& current = heap.extractMin();
            Vertex currentVertex = current.getVertex();

            for(const Edge& edge : graph.getEdges(currentVertex, direction))
            {
              Vertex nextVertex = edge.getEndpoint(direction);
              const num nextCost = current.getCost() +
                reducedCosts(edge);

              Label nextLabel = Label(nextVertex, edge, nextCost);

              heap.update(nextLabel);
            }
          }
        }
      }

      VertexSet considered(graph);

      for(const Vertex& vertex : region.getVertices())
      {
        considered.insert(vertex);
      }

      for(auto rit : it.second)
      {
        const Region& otherRegion = *rit;

        for(const Vertex& target : otherRegion.getVertices())
        {
          Label current = heap.getLabel(target);

          assert(current.getState() == State::SETTLED);

          while(!(considered.contains(current.getVertex())))
          {
            Edge edge = current.getEdge();
            flaggedEdges.push_back(edge);
            considered.insert(current.getVertex());
            current = heap.getLabel(edge.getEndpoint(opposite(direction)));
          }
        }
      }
    }

    {
      tbb::spin_mutex::scoped_lock lock(mutex);

      for(const Edge& edge : flaggedEdges)
      {
        flags.extend(edge, region, value);
      }
    }
  }

  {
    tbb::spin_mutex::scoped_lock lock(mutex);

    for(const Edge& edge : partition.internalEdges(region))
    {
      flags.extend(edge, region);
    }
  }
}


#endif /* VALUE_ARCFLAG_PREPROCESSOR_HH */

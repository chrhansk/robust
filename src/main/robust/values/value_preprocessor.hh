#ifndef VALUE_PREPROCESSOR_HH
#define VALUE_PREPROCESSOR_HH

#include <unordered_set>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "arcflags/partition.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "abstract_value_preprocessor.hh"

/**
 * A class which computes the set of \f$ \theta \f$-values
 * which are required for any robust shortest Path between
 * two Region%s of a given Partition.
 **/
class ValuePreprocessor : public AbstractValuePreprocessor
{
private:

  template<Direction direction, class Func>
  void computeShortestPaths(const Region& region,
                            std::unordered_set<Vertex>& boundaryVertices,
                            Func func) const;

public:
  ValuePreprocessor(const Graph& graph,
                    const EdgeFunc<num>& costs,
                    const EdgeFunc<num>& deviations,
                    num deviationSize,
                    const Partition& partition)
    : AbstractValuePreprocessor(graph, costs, deviations, deviationSize, partition)
  {}

  std::unordered_set<num> requiredValues(const Region& sourceRegion,
                                         const Region& targetRegion) const;

};

template<Direction direction, class Func>
void ValuePreprocessor::computeShortestPaths(const Region& region,
                                             std::unordered_set<Vertex>& boundaryVertices,
                                             Func func) const
{
  for(const Vertex& boundaryVertex : boundaryVertices)
  {
    auto filter = partition.regionFilter(region);

    for(const num& value : values)
    {
      ReducedCosts reducedCosts(costs, deviations, value);

      LabelHeap<Label> heap(graph);

      heap.update(Label(boundaryVertex, Edge(), 0));

      for(const Vertex& vertex : region.getVertices())
      {
        while(!heap.isEmpty())
        {
          const Label& current = heap.extractMin();

          if(heap.getLabel(vertex).getState() == State::SETTLED)
          {
            break;
          }

          for(const Edge& edge : graph.getEdges(current.getVertex(), opposite(direction)))
          {
            if(!filter(edge))
            {
              continue;
            }

            Label nextLabel = Label(edge.getEndpoint(opposite(direction)),
                                    edge, current.getCost() + reducedCosts(edge));

            heap.update(nextLabel);
          }
        }

        if(direction == Direction::OUTGOING)
        {
          func(vertex, boundaryVertex, heap.getLabel(vertex).getCost());
        }
        else
        {
          func(boundaryVertex, vertex, heap.getLabel(vertex).getCost());
        }
      }
    }
  }
}


#endif /* VALUE_PREPROCESSOR_HH */

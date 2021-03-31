#include "refining_value_preprocessor.hh"

#include "log.hh"

#include "graph/vertex_set.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

#include "robust/reduced_costs.hh"

#include "outer_value_preprocessor.hh"

ValueSet
RefiningValuePreprocessor::requiredValues(const Region& sourceRegion,
                                          const Region& targetRegion,
                                          const ValueSet& possibleValues) const
{
  ValueSet requiredValues;

  Boundary sourceBoundary = partition.getBoundary<Direction::OUTGOING>(sourceRegion);
  Boundary targetBoundary = partition.getBoundary<Direction::INCOMING>(targetRegion);

  DistanceMap boundaryDistances;

  Log(info) << "Computing boundary distances for "
            << values.size()
            << " values by computing "
            << sourceBoundary.getVertices().size()
            << " shortest path trees";

  /*
   * Add all occurring values to the required values.
   *
   */
  {
    for(const Edge& edge : partition.internalEdges(sourceRegion))
    {
      requiredValues.insert(deviations(edge));
    }

    for(const Edge& edge : partition.internalEdges(targetRegion))
    {
      requiredValues.insert(deviations(edge));
    }
  }

  for(const Vertex& source : sourceBoundary.getVertices())
  {
    for(const Vertex& target : targetBoundary.getVertices())
    {
      boundaryDistances.put(source, target , {});
    }

    for(const num& value : possibleValues)
    {

      ReducedCosts reducedCosts(costs, deviations, value);

      /*
       * Explore all targets (vertices in the target boundary).
       */
      {

        LabelHeap<Label> heap(graph);

        heap.update(Label(source, Edge(), 0));

        for(const Vertex& target : targetBoundary.getVertices())
        {
          while(!heap.isEmpty())
          {
            if(heap.getLabel(target).getState() == State::SETTLED)
            {
              boundaryDistances(source, target).push_back(heap.getLabel(target).getCost());
              break;
            }

            const Label& current = heap.extractMin();

            for(const Edge& edge : graph.getOutgoing(current.getVertex()))
            {
              Label nextLabel = Label(edge.getTarget(),
                                      edge,
                                      current.getCost() + reducedCosts(edge));

              heap.update(nextLabel);
            }
          }
        }

        VertexSet considered(graph);

        considered.insert(source);

        /*
         * Walk back from individual targets (vertices in the target boundary)
         * to the current source vertex. Add the all occurring values to
         * the possible values.
         */
        for(const Vertex& target : targetBoundary.getVertices())
        {
          Label current = heap.getLabel(target);

          assert(current.getState() == State::SETTLED);

          while(!(considered.contains(current.getVertex())))
          {
            Edge edge = current.getEdge();

            num deviation = deviations(edge);

            if(deviation < value)
            {
              requiredValues.insert(deviation);
            }

            considered.insert(current.getVertex());

            current = heap.getLabel(edge.getSource());
          }
        }
      }
    }
  }

  OuterValuePreprocessor preprocessor(graph,
                                      costs,
                                      deviations,
                                      deviationSize,
                                      partition);

  return preprocessor.requiredValues(sourceRegion,
                                     targetRegion,
                                     boundaryDistances,
                                     requiredValues);
}

RegionMap<ValueSet>
RefiningValuePreprocessor::requiredValues(const Region& sourceRegion) const
{
  RegionMap<ValueSet> possibleValues(partition, {}),
    requiredValues(partition, {});

  auto sourceEdges = partition.internalEdges(sourceRegion);

  VertexSet boundaryVertices(graph);

  const Boundary sourceBoundary =
    partition.getBoundary<Direction::OUTGOING>(sourceRegion);

  DistanceMap boundaryDistances;

  for(const Region& targetRegion : partition.getRegions())
  {
    if(sourceRegion == targetRegion)
    {
      continue;
    }

    auto targetEdges = partition.internalEdges(targetRegion);

    for(const Edge& edge : sourceEdges)
    {
      possibleValues(targetRegion).insert(deviations(edge));
    }

    for(const Edge& edge : targetEdges)
    {
      possibleValues(targetRegion).insert(deviations(edge));
    }

    Boundary targetBoundary =
      partition.getBoundary<Direction::INCOMING>(targetRegion);

    for(const Vertex& source : sourceBoundary.getVertices())
    {
      for(const Vertex& target : targetBoundary.getVertices())
      {
        boundaryDistances.put(source, target, {});
      }
    }

    for(const Vertex& target : targetBoundary.getVertices())
    {
      boundaryVertices.insert(target);
    }

  }

  Log(info) << "Computing boundary distances for "
            << values.size()
            << " values using "
            << sourceBoundary.getVertices().size()
            << " shortest path trees";

  for(const Vertex& source : sourceBoundary.getVertices())
  {
    Log(info) << "Computing " << values.size()
              << " shortest path trees";

    int c = 0;
    for(const num& value : values)
    {
      Log(info) << "Computing tree " << ++c
                << " of " << values.size();

      ReducedCosts reducedCosts(costs, deviations, value);

      LabelHeap<Label> heap(graph);

      heap.update(Label(source, Edge(), 0));

      for(const Vertex& vertex : graph.getVertices())
      {
        if(!boundaryVertices.contains(vertex))
        {
          continue;
        }

        while(!heap.isEmpty())
        {
          if(heap.getLabel(vertex).getState() == State::SETTLED)
          {
            break;
          }

          const Label& current = heap.extractMin();

          for(const Edge& edge : graph.getOutgoing(current.getVertex()))
          {
            Label nextLabel = Label(edge.getTarget(),
                                    edge,
                                    current.getCost() + reducedCosts(edge));

            heap.update(nextLabel);
          }
        }
      }

      for(const Region& targetRegion : partition.getRegions())
      {
        if(sourceRegion == targetRegion)
        {
          continue;
        }

        const Boundary targetBoundary =
          partition.getBoundary<Direction::INCOMING>(targetRegion);

        for(const Vertex& target : targetBoundary.getVertices())
        {
          const Label& label = heap.getLabel(target);

          assert(label.getState() == State::SETTLED);

          boundaryDistances(source, target).push_back(label.getCost());
        }

        VertexSet considered(graph);
        considered.insert(source);

        for(const Vertex& target : targetBoundary.getVertices())
        {
          Label current = heap.getLabel(target);

          assert(current.getState() == State::SETTLED);

          while(!(considered.contains(current.getVertex())))
          {
            Edge edge = current.getEdge();

            num deviation = deviations(edge);

            if(deviation < value)
            {
              possibleValues(targetRegion).insert(deviation);
            }

            considered.insert(current.getVertex());

            current = heap.getLabel(edge.getSource());
          }
        }
      }
    }
  }

  OuterValuePreprocessor preprocessor(graph,
                                      costs,
                                      deviations,
                                      deviationSize,
                                      partition);

  for(const Region& targetRegion : partition.getRegions())
  {
    if(sourceRegion == targetRegion)
    {
      continue;
    }

    requiredValues(targetRegion) = possibleValues(targetRegion);

    /*
    requiredValues(targetRegion) = preprocessor.requiredValues(sourceRegion,
                                                               targetRegion,
                                                               boundaryDistances,
                                                               possibleValues(targetRegion));
    */
  }

  Log(info) << "Done computing required values";

  return requiredValues;
}



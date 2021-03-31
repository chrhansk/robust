#include "fast_arcflag_preprocessor.hh"

#include <iterator>

#include <tbb/tbb.h>

#include "log.hh"
#include "graph/vertex_set.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"
#include "robust/value_range.hh"

namespace
{
  class SynchronizedExtender
  {
  private:

    class Extension
    {
    public:
      Edge edge;
      const Region& region;
      num minValue;
      num maxValue;

      Extension(const Edge& edge,
                const Region& region,
                num minValue,
                num maxValue)
        : edge(edge),
          region(region),
          minValue(minValue),
          maxValue(maxValue)
      {}
    };

  private:
    tbb::spin_mutex& mutex;
    RobustArcFlags& flags;
    std::vector<Extension> extensions;

  public:
    SynchronizedExtender(tbb::spin_mutex& mutex,
                         RobustArcFlags& flags)
      : mutex(mutex),
        flags(flags)
    {}

    void operator()(const Edge& edge,
                    const Region& region,
                    num minValue,
                    num maxValue)
    {
      extensions.push_back(Extension(edge, region, minValue, maxValue));
    }

    ~SynchronizedExtender()
    {
      tbb::spin_mutex::scoped_lock lock(mutex);

      for(const Extension& extension : extensions)
      {
        flags.extend(extension.edge,
                     extension.region,
                     extension.minValue,
                     extension.maxValue);
      }
    }
  };

}

namespace
{
  class Interval
  {
  private:
    ValueRange range;
    num minValue;
    num maxValue;
    std::vector<Edge> additionalEdges;
    const EdgeFunc<num>* deviations;

  public:
    Interval(const ValueVector& values,
             const std::vector<Edge>& additionalEdges,
             const EdgeFunc<num>& deviations)
      : range(ValueRange(values).innerRange()),
        minValue(*values.rbegin()),
        maxValue(*values.begin()),
        additionalEdges(additionalEdges),
        deviations(&deviations)
    {
      assert(check());
    }

    Interval(ValueRange range,
             num minValue,
             num maxValue,
             const std::vector<Edge>& additionalEdges,
             const EdgeFunc<num>& deviations)
      : range(range),
        minValue(minValue),
        maxValue(maxValue),
        additionalEdges(additionalEdges),
        deviations(&deviations)
    {
      assert(check());
    }

    const EdgeFunc<num>& getDeviations() const
    {
      return *deviations;
    }

    num getMinValue() const
    {
      return minValue;
    }

    num getMaxValue() const
    {
      return maxValue;
    }

    bool check() const
    {
      if(minValue > maxValue)
      {
        return false;
      }

      for(const Edge& edge : additionalEdges)
      {
        if(getDeviations()(edge) <= minValue or
           getDeviations()(edge) >= maxValue)
        {
          return false;
        }
      }

      return true;
    }

    ValueRange getRange() const
    {
      return range;
    }

    const std::vector<Edge>& getAdditionalEdges() const
    {
      return additionalEdges;
    }

    std::vector<Edge>& getAdditionalEdges()
    {
      return additionalEdges;
    }

    std::pair<Interval, Interval>
    split(ValueIterator middle,
          const std::vector<Edge>& lowerEdges)
    {
      num middleValue = *middle;
      auto ranges = range.split(middle);

      std::vector<Edge> upperEdges;

      for(const Edge& edge : additionalEdges)
      {
        if(getDeviations()(edge) > middleValue)
        {
          upperEdges.push_back(edge);
        }
      }

      return std::make_pair(Interval(ranges.first,
                                     middleValue,
                                     maxValue,
                                     upperEdges,
                                     *deviations),
                            Interval(ranges.second,
                                     minValue,
                                     middleValue,
                                     lowerEdges,
                                     *deviations));
    }

    bool operator<(const Interval& other) const
    {
      return getAdditionalEdges().size() < other.getAdditionalEdges().size();
    }
  };

  static num treeCost(const LabelHeap<Label> &heap,
                      Vertex root,
                      Vertex vertex,
                      Direction direction,
                      const EdgeFunc<num>& costs)
  {
    num sum = (num) 0;

    while(vertex != root)
    {
      const Label& label = heap.getLabel(vertex);
      const Edge& edge = label.getEdge();
      sum += costs(edge);
      vertex = edge.getEndpoint(opposite(direction));
    }

    return sum;
  }

}

FastArcFlagPreprocessor::FastArcFlagPreprocessor(const Graph& graph,
                                                 const EdgeFunc<num>& costs,
                                                 const EdgeFunc<num>& deviations,
                                                 const Partition& partition)
  : AbstractArcFlagPreprocessor(graph, costs, deviations, partition)
{
}

template <Direction direction, class OutIt>
void FastArcFlagPreprocessor::computeTree(Vertex root,
                                          const Region& region,
                                          const EdgeFunc<num>& reducedCosts,
                                          LabelHeap<Label>& heap,
                                          OutIt outIt) const
{
  heap.update(Label(root, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();
    Vertex currentVertex = current.getVertex();

    if(current.getVertex() != root)
    {
      *outIt++ = current.getEdge();

      if(region == partition.getRegion(current.getVertex()))
      {
        continue;
      }
    }

    for(const Edge& edge : graph.getEdges(currentVertex,
                                          direction))
    {
      Vertex nextVertex = edge.getEndpoint(direction);
      const num nextCost = current.getCost() + reducedCosts(edge);

      Label nextLabel = Label(nextVertex, edge, nextCost);

      heap.update(nextLabel);
    }
  }
}

template <Direction direction, class Extend>
void FastArcFlagPreprocessor::setFlags(Vertex vertex,
                                       idx numTrees,
                                       Extend extend) const
{
  num minValue = *(values.rbegin());
  num maxValue = *(values.begin());
  std::unordered_set<Edge> edges;
  std::vector<Edge> additionalEdges;
  const Region& region = partition.getRegion(vertex);

  {
    LabelHeap<Label> heap(graph);

    computeTree<direction>(vertex,
                           region,
                           ReducedCosts(costs, deviations, maxValue),
                           heap,
                           std::inserter(edges, edges.end()));

    collectAdditionalEdges<direction>(vertex,
                                      heap,
                                      region,
                                      edges,
                                      minValue,
                                      maxValue,
                                      std::back_inserter(additionalEdges));

  }

  {
    LabelHeap<Label> heap(graph);

    computeTree<direction>(vertex,
                           region,
                           ReducedCosts(costs, deviations, minValue),
                           heap,
                           std::inserter(edges, edges.end()));
  }

  idx numComputations = 2;

  std::priority_queue<Interval> intervals;

  intervals.push(Interval(values, additionalEdges, deviations));

  while(numComputations < numTrees)
  {
    if(intervals.top().getAdditionalEdges().empty())
    {
      break;
    }

    Interval interval = intervals.top();
    intervals.pop();

    ValueRange valueRange = interval.getRange();
    ValueIterator middle = valueRange.middle();
    num middleValue = *middle;

    LabelHeap<Label> heap(graph);

    ++numComputations;

    computeTree<direction>(vertex,
                           region,
                           ReducedCosts(costs, deviations, middleValue),
                           heap,
                           std::inserter(edges, edges.end()));

    std::vector<Edge> lowerEdges;

    collectAdditionalEdges<direction>(vertex,
                                      heap,
                                      region,
                                      edges,
                                      interval.getMinValue(),
                                      middleValue,
                                      std::back_inserter(lowerEdges));

    auto pair = interval.split(middle, lowerEdges);

    intervals.push(pair.first);
    intervals.push(pair.second);
  }

  for(const Edge& edge : edges)
  {
    extend(edge, region, minValue, maxValue);
  }

  while(!intervals.empty())
  {
    const Interval& interval = intervals.top();
    for(const Edge& edge : interval.getAdditionalEdges())
    {
      extend(edge,
             region,
             interval.getMinValue(),
             interval.getMaxValue());
    }

    intervals.pop();
  }
}

template<Direction direction,
         class OutIt>
void FastArcFlagPreprocessor::collectAdditionalEdges(Vertex root,
                                                     const LabelHeap<Label>& heap,
                                                     const Region& region,
                                                     const std::unordered_set<Edge>& edges,
                                                     num minValue,
                                                     num maxValue,
                                                     OutIt outIt) const
{
  ReducedCosts reducedCosts(costs, deviations, maxValue);

  for(const Edge& edge : graph.getEdges())
  {
    const Vertex& source = edge.getEndpoint(opposite(direction));
    const Vertex& target = edge.getEndpoint(direction);

    if(edges.find(edge) == edges.end())
    {
      continue;
    }

    if(deviations(edge) <= minValue or deviations(edge) >= maxValue)
    {
      continue;
    }

    if(!(heap.getLabel(source).getState() == State::SETTLED and
         heap.getLabel(target).getState() == State::SETTLED))
    {
      continue;
    }

    num sourceCost = treeCost(heap,
                              root,
                              source,
                              direction,
                              reducedCosts);

    {
      ReducedCosts currentCosts = ReducedCosts(costs,
                                               deviations,
                                               minValue);

      num targetCost = treeCost(heap,
                                root,
                                target,
                                direction,
                                currentCosts);

      if(sourceCost + currentCosts(edge) < targetCost)
      {
        *outIt++ = edge;
        continue;
      }
    }

    {
      ReducedCosts currentCosts = ReducedCosts(costs,
                                               deviations,
                                               deviations(edge));

      num targetCost = treeCost(heap,
                                root,
                                target,
                                direction,
                                currentCosts);

      if(sourceCost + currentCosts(edge) < targetCost)
      {
        *outIt++ = edge;
      }
    }
  }
}

void FastArcFlagPreprocessor::computeFlags(RobustArcFlags& incomingFlags,
                                           RobustArcFlags& outgoingFlags,
                                           idx numTrees,
                                           bool parallelComputation) const
{
  std::vector<Edge> overlappingEdges;

  Log(info) << "Computing arc flags for " << values.size()
            << " values using " << numTrees
            << " trees [parallel = "
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
    tbb::spin_mutex mutex;
    tbb::parallel_do(sourceVertices.begin(),
                     sourceVertices.end(),
                     [this, &mutex, &outgoingFlags, numTrees](const Vertex& vertex)
                     {
                       SynchronizedExtender extender(mutex, outgoingFlags);
                       setFlags<Direction::OUTGOING>(vertex,
                                                     numTrees,
                                                     extender);
                     });

    tbb::parallel_do(targetVertices.begin(),
                     targetVertices.end(),
                     [this, &mutex, &incomingFlags, numTrees](const Vertex& vertex)
                     {
                       SynchronizedExtender extender(mutex, incomingFlags);
                       setFlags<Direction::INCOMING>(vertex,
                                                     numTrees,
                                                     extender);
                     });
  }
  else
  {
    for(const Vertex& vertex : sourceVertices)
    {
      setFlags<Direction::OUTGOING>(vertex, numTrees,
                                    [&](const Edge& edge, const Region& region, num minValue, num maxValue)
                                    {
                                      outgoingFlags.extend(edge, region, minValue, maxValue);
                                    });
    }

    for(const Vertex& vertex : targetVertices)
    {
      setFlags<Direction::INCOMING>(vertex, numTrees,
                                    [&](const Edge& edge, const Region& region, num minValue, num maxValue)
                                    {
                                      incomingFlags.extend(edge, region, minValue, maxValue);
                                    });
    }
  }
}

#include "robust_contraction_preprocessor.hh"

#include <algorithm>
#include <iterator>
#include <queue>
#include <stack>
#include <unordered_map>

#include <boost/heap/d_ary_heap.hpp>

#include "log.hh"

#include "robust/reduced_costs.hh"
#include "robust/robust_utils.hh"

#include "value_count_quotient.hh"
#include "value_range_quotient.hh"

#include "fast_robust_witness_path_search.hh"
#include "simple_robust_witness_path_search.hh"

typedef typename boost::heap::d_ary_heap<Vertex,
                                         boost::heap::mutable_<true>,
                                         boost::heap::compare<VertexCompare<float, true>>,
                                         boost::heap::arity<2>> Queue;

typedef typename Queue::handle_type Handle;

RobustContractionPreprocessor::RobustContractionPreprocessor(const Graph& graph,
                                                             const EdgeFunc<num>& costs,
                                                             const EdgeFunc<num>& deviations)
  : AbstractRobustContractionPreprocessor(graph, costs, deviations)
{
}


RobustContractionHierarchy RobustContractionPreprocessor::computeHierarchy() const
{
  Graph overlayGraph(graph);
  EdgeMap<ContractionRange> contractionRanges(graph, ContractionRange());
  VertexMap<num> rankMap(overlayGraph, INVALID);
  EdgeMap<EdgePair> originalEdges(overlayGraph, EdgePair());

  idx currentRank = 0;

  Contracted contracted(rankMap);

  Log(info) << "Total number of values: "
            << values.size();


  for(const Edge& edge : overlayGraph.getEdges())
  {
    contractionRanges(edge) = ContractionRange(values.begin(),
                                               values.end(),
                                               costs(edge));
    originalEdges(edge) = EdgePair(edge);
    contractionRanges(edge).getValues().push_back(deviations(edge));
  }

  tightenEdges(overlayGraph, contractionRanges);

  SimpleRobustWitnessPathSearch search(overlayGraph,
                                       contractionRanges,
                                       values,
                                       contracted);

  /*
  SumFunction<ValueRangeQuotient, ValueCountQuotient> scoreFunc(
    ValueRangeQuotient(overlayGraph, contractionRanges),
    ValueCountQuotient(overlayGraph, contractionRanges));
  */

  ValueRangeQuotient scoreFunc(overlayGraph, contractionRanges);
  //ValueCountQuotient scoreFunc(overlayGraph, contractionRanges);

  VertexMap<float> scoreMap(overlayGraph, 0);

  for(const Vertex& vertex : overlayGraph.getVertices())
  {
    std::vector<RobustContractionPair> pairs = search.findPairs(vertex);
    assert(!contracted(vertex));

    if(debuggingEnabled())
    {
      for(auto pair : pairs)
      {
        assert(pair.check(overlayGraph, values, contracted, contractionRanges));
      }
    }

    float score = scoreFunc.getScore(vertex, pairs);
    scoreMap(vertex) = score;
  }

  VertexValueMap<float> scoreValues = scoreMap.getValues();
  VertexMap<Handle> handles(graph, Handle());
  VertexCompare<float, true> scoreCompare(scoreValues);
  Queue queue(scoreCompare);

  for(const Vertex& vertex : overlayGraph.getVertices())
  {
    handles(vertex) = queue.push(vertex);
  }

  auto scoreUpdate = [&](const Vertex& vertex, const std::vector<RobustContractionPair>& pairs) -> bool
    {
      float expectedScore = scoreMap(vertex);
      float actualScore = scoreFunc.getScore(vertex, pairs);

      if(expectedScore != actualScore)
      {
        scoreMap(vertex) = actualScore;
        queue.erase(handles(vertex));
        handles(vertex) = queue.push(vertex);
        return true;
      }

      return false;
    };

  while(!queue.empty())
  {
    //int numEdges = overlayGraph.getEdges().size();

    Vertex vertex = queue.top();

    assert(!contracted(vertex));

    std::vector<RobustContractionPair> pairs = search.findPairs(vertex);

    if(scoreUpdate(vertex, pairs))
    {
      continue;
    }

    queue.pop();


    contractVertex(vertex,
                   overlayGraph,
                   originalEdges,
                   contractionRanges,
                   contracted,
                   pairs);

    rankMap(vertex) = currentRank++;
    assert(contracted(vertex));

    for(const Edge& edge : graph.getAdjacentEdges(vertex))
    {
      const Vertex other = edge.getOpposite(vertex);

      if(contracted(other))
      {
        continue;
      }

      std::vector<RobustContractionPair> pairs = search.findPairs(other);

      scoreUpdate(other, pairs);
    }
  }

  for(const Vertex& vertex : overlayGraph.getVertices())
  {
    assert(contracted(vertex));
  }

  removeOverflows(overlayGraph, contractionRanges);

  Log(info) << "Contraction increased #edges from "
             << graph.getEdges().size()
             << " to "
             << overlayGraph.getEdges().size();

  std::unordered_map<num, idx> edgeCount;

  for(const num& value : values)
  {
    edgeCount[value] = 0;
  }

  for(const Edge& edge : overlayGraph.getEdges())
  {
    const ContractionRange& range = contractionRanges(edge);

    for(auto it = range.getBegin(); it != range.getEnd(); ++it)
    {
      ++edgeCount[*it];
    }
  }

  idx totalSize = 0;
  idx minSize = overlayGraph.getEdges().size();
  idx maxSize = 0;

  for(const num& value : values)
  {
    idx current = edgeCount[value];
    totalSize += current;
    minSize = std::min(minSize, current);
    maxSize = std::max(maxSize, current);
  }

  Log(info) << "Min size: "
             << minSize
             << ", max size: "
             << maxSize
             << ", avg size: "
             << totalSize / ((float) values.size());

  idx totalValues = 0;

  for(const Edge& edge : overlayGraph.getEdges())
  {
    totalValues += contractionRanges(edge).getValues().size();
  }

  Log(info) << "Total number of values: "
             << totalValues;

  return RobustContractionHierarchy(overlayGraph,
                                    contractionRanges,
                                    rankMap,
                                    originalEdges);

}

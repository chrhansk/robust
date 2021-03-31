#include "contraction_preprocessor.hh"

#include "log.hh"

#include "graph/vertex_map.hh"

#include "fast_witness_path_search.hh"
#include "hop_restricted_witness_path_search.hh"
#include "simple_witness_path_search.hh"

#include "edge_count.hh"
#include "edge_quotient.hh"
#include "level_estimation.hh"

ContractionPreprocessor::ContractionPreprocessor(const Graph& graph,
                                                 const EdgeFunc<num>& costs)
  : AbstractContractionPreprocessor(graph, costs)
{
}

ContractionHierarchy ContractionPreprocessor::computeHierarchy()
{
  Graph overlayGraph(graph);
  EdgeMap<num> overlayCosts(overlayGraph, (num) 0);
  VertexMap<num> rankMap(overlayGraph, INVALID);
  EdgeMap<EdgePair> originalEdges(overlayGraph, EdgePair());

  int currentRank = 0;

  Contracted contracted(rankMap);

  EdgeValueMap<num> costValues = overlayCosts.getValues();

  //SimpleWitnessPathSearch search(overlayGraph, costValues, contracted);
  FastWitnessPathSearch search(overlayGraph, costValues, contracted);
  //HopRestrictedWitnessPathSearch search(*this, 10);
  //EdgeQuotient scoreFunc(*this, search);

  auto scoreFunc = SumFunction<EdgeQuotient,
                               EdgeCount,
                               LevelEstimation>(EdgeQuotient(overlayGraph),
                                                EdgeCount(overlayGraph),
                                                LevelEstimation(overlayGraph));

  VertexMap<float> scoreMap(overlayGraph, 0);

  for(const Edge& edge : overlayGraph.getEdges())
  {
    overlayCosts(edge) = costs(edge);
    originalEdges(edge) = EdgePair(edge);
  }

  for(const Vertex& vertex : overlayGraph.getVertices())
  {
    auto pairs = search.findPairs(vertex);
    assert(!contracted(vertex));
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

  auto scoreUpdate = [&](const Vertex& vertex, const std::vector<ContractionPair>& pairs) -> bool
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
    const Vertex vertex = queue.top();

    assert(!contracted(vertex));

    std::vector<ContractionPair> pairs = search.findPairs(vertex);

    if(scoreUpdate(vertex, pairs))
    {
      continue;
    }

    queue.pop();

    rankMap(vertex) = currentRank++;
    auto results = contractVertex(vertex,
                                  overlayGraph,
                                  overlayCosts,
                                  originalEdges,
                                  contracted,
                                  pairs);

    scoreFunc.vertexContracted(vertex, results);

    assert(contracted(vertex));

    for(const Edge& edge : graph.getAdjacentEdges(vertex))
    {
      const Vertex other = edge.getOpposite(vertex);

      if(contracted(other))
      {
        continue;
      }

      std::vector<ContractionPair> pairs = search.findPairs(other);

      scoreUpdate(other, pairs);
    }
  }

  Log(info) << "Contraction increased #edges from "
             << graph.getEdges().size()
             << " to "
             << overlayGraph.getEdges().size();

  return ContractionHierarchy(overlayGraph,
                              costValues,
                              rankMap,
                              originalEdges.getValues());
}

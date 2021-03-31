#include "parallel_contraction_preprocessor.hh"

#include <tbb/tbb.h>

#include "graph/vertex_set.hh"

#include "fast_witness_path_search.hh"
#include "hop_restricted_witness_path_search.hh"
#include "simple_witness_path_search.hh"

#include "edge_count.hh"
#include "edge_quotient.hh"
#include "level_estimation.hh"

typedef std::pair<Vertex,
                  std::vector<ContractionPair>> ContractionVertex;


bool ParallelContractionPreprocessor::isMinimal(const Graph& graph,
                                                const Vertex& vertex,
                                                idx neighborhoodSize,
                                                const ParallelContractionPreprocessor::Contracted& contracted,
                                                const ParallelContractionPreprocessor::ScoreComparator& comparator)
{
  return graph.visitNeighbors(vertex, neighborhoodSize,
                              [&] (const Vertex& other)
                              {
                                if(!contracted(other) and
                                   other != vertex and
                                   !comparator(vertex, other))
                                {
                                  return false;
                                }

                                return true;
                              });
}


ParallelContractionPreprocessor::ParallelContractionPreprocessor(
  const Graph& graph,
  const EdgeFunc<num>& costs,
  idx neighborhoodSize)
  : AbstractContractionPreprocessor(graph, costs),
    neighborhoodSize(neighborhoodSize)
{
  assert(neighborhoodSize >= 2);
}

ContractionHierarchy ParallelContractionPreprocessor::computeHierarchy()
{
  Graph overlayGraph(graph);
  EdgeMap<num> overlayCosts(overlayGraph, (num) 0);
  VertexMap<num> rankMap(overlayGraph, INVALID);
  EdgeMap<EdgePair> originalEdges(overlayGraph, EdgePair());

  idx currentRank = 0;

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

  tbb::spin_mutex mutex;
  ScoreComparator comparator(scoreMap);

  std::vector<Vertex> vertices = overlayGraph.getVertices().collect();

  std::cerr << "Computing initial scores" << std::endl;

  tbb::parallel_do(vertices.begin(),
                   vertices.end(),
                   [&](const Vertex& vertex)
                   {
                     auto pairs = search.findPairs(vertex);
                     assert(!contracted(vertex));

                     {
                       tbb::spin_mutex::scoped_lock lock(mutex);
                       float score = scoreFunc.getScore(vertex, pairs);
                       scoreMap(vertex) = score;
                     }
                   });

  std::cerr << "Done" << std::endl;

  while(!vertices.empty())
  {
    std::vector<ContractionVertex> nextVertices;
    std::vector<Vertex> discardedVertices;

    std::cerr << "Computing minima for "
      << vertices.size()
      << " vertices"
      << std::endl;

    tbb::parallel_do(vertices.begin(),
                     vertices.end(),
                     [&](const Vertex& vertex)
                     {
                       assert(!contracted(vertex));

                       if(isMinimal(overlayGraph,
                                    vertex,
                                    neighborhoodSize,
                                    contracted,
                                    comparator))
                       {
                         auto pairs = search.findPairs(vertex);
                         {
                           tbb::spin_mutex::scoped_lock lock(mutex);
                           nextVertices.push_back(std::make_pair(vertex, pairs));
                         }
                       }
                       else
                       {
                         tbb::spin_mutex::scoped_lock lock(mutex);
                         discardedVertices.push_back(vertex);
                       }

                     });

    std::cerr << "Found " << nextVertices.size() << " minima" << std::endl;

    for(auto pair : nextVertices)
    {
      const Vertex& vertex = pair.first;

      rankMap(vertex) = currentRank++;
      auto results = contractVertex(vertex,
                                    overlayGraph,
                                    overlayCosts,
                                    originalEdges,
                                    contracted,
                                    pair.second);

      scoreFunc.vertexContracted(vertex, results);
    }

    tbb::parallel_do(nextVertices.begin(),
                     nextVertices.end(),
                     [&](const ContractionVertex& contractionVertex)
                     {
                       const Vertex& vertex = contractionVertex.first;

                       for(const Edge& edge : graph.getAdjacentEdges(vertex))
                       {
                         Vertex other = edge.getOpposite(vertex);

                         if(contracted(other))
                         {
                           continue;
                         }

                         auto pairs = search.findPairs(other);

                         {
                           tbb::spin_mutex::scoped_lock lock(mutex);
                           float score = scoreFunc.getScore(other, pairs);
                           scoreMap(other) = score;
                         }
                       }
                     });


    vertices = discardedVertices;
  }

  std::cerr << "Contraction increased #edges from "
            << graph.getEdges().size()
            << " to "
            << overlayGraph.getEdges().size()
            << std::endl;

  return ContractionHierarchy(overlayGraph,
                              costValues,
                              rankMap,
                              originalEdges.getValues());
}

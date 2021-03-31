#include "parallel_robust_contraction_preprocessor.hh"

#include <unordered_map>

#include <tbb/tbb.h>

#include "value_count_quotient.hh"
#include "value_range_quotient.hh"

#include "fast_robust_witness_path_search.hh"
#include "simple_robust_witness_path_search.hh"

typedef std::pair<Vertex,
                  std::vector<RobustContractionPair>> ContractionVertex;

bool ParallelRobustContractionPreprocessor::isMinimal(
  const Graph& graph,
  const Vertex& vertex,
  idx neighborhoodSize,
  const ParallelRobustContractionPreprocessor::Contracted& contracted,
  const ParallelRobustContractionPreprocessor::ScoreComparator& comparator) const
{
  return graph.visitNeighbors(vertex, neighborhoodSize,
                              [&] (const Vertex& other) -> bool
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

RobustContractionHierarchy
ParallelRobustContractionPreprocessor::computeHierarchy() const
{
  Graph overlayGraph(graph);
  EdgeMap<ContractionRange> contractionRanges(graph, ContractionRange());
  VertexMap<num> rankMap(overlayGraph, INVALID);
  EdgeMap<EdgePair> originalEdges(overlayGraph, EdgePair());

  idx currentRank = 0;

  Contracted contracted(rankMap);

  std::cerr << "Total number of values: "
            << values.size()
            << std::endl;


  for(const Edge& edge : overlayGraph.getEdges())
  {
    contractionRanges(edge) = ContractionRange(values.begin(),
                                               values.end(),
                                               costs(edge));
    originalEdges(edge) = EdgePair(edge);
    contractionRanges(edge).getValues().push_back(deviations(edge));
  }

  tightenEdges(overlayGraph, contractionRanges);

  FastRobustWitnessPathSearch search(overlayGraph,
                                     contractionRanges,
                                     values,
                                     contracted);

  ValueRangeQuotient scoreFunc(overlayGraph, contractionRanges);

  tbb::spin_mutex mutex;

  VertexMap<float> scoreMap(overlayGraph, 0);
  ScoreComparator comparator(scoreMap);

  auto vertices = overlayGraph.getVertices().collect();

  std::cerr << "Computing initial scores" << std::endl;

  tbb::parallel_do(vertices.begin(),
                   vertices.end(),
                   [&](const Vertex& vertex)
                   {
                     std::vector<RobustContractionPair> pairs =
                       search.findPairs(vertex);
                     assert(!contracted(vertex));

                     if(debuggingEnabled())
                     {
                       for(auto pair : pairs)
                       {
                         assert(pair.check(overlayGraph,
                                           values,
                                           contracted,
                                           contractionRanges));
                       }
                     }

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
      contractVertex(vertex,
                     overlayGraph,
                     originalEdges,
                     contractionRanges,
                     contracted,
                     pair.second);

      //scoreFunc.vertexContracted(vertex, results);
    }

    tbb::parallel_do(nextVertices.begin(),
                     nextVertices.end(),
                     [&](const ContractionVertex& contractionVertex)
                     {
                       Vertex vertex = contractionVertex.first;

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

  removeOverflows(overlayGraph, contractionRanges);

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

  std::cerr << "Min size: "
            << minSize
            << ", max size: "
            << maxSize
            << ", avg size: "
            << totalSize / ((float) values.size())
            << std::endl;

  idx totalValues = 0;

  for(const Edge& edge : overlayGraph.getEdges())
  {
    totalValues += contractionRanges(edge).getValues().size();
  }

  std::cerr << "Total number of values: "
            << totalValues << std::endl;

  return RobustContractionHierarchy(overlayGraph,
                                    contractionRanges,
                                    rankMap,
                                    originalEdges);
}

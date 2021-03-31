#include "arcflag_test.hh"

#include <gtest/gtest.h>

#include "log.hh"

#include "graph/graph.hh"
#include "router/router.hh"

#include "arcflags/centralized_preprocessor.hh"
#include "arcflags/metis_partition.hh"

/*
TEST_F(ArcFlagTest, testPartition)
{
  ASSERT_TRUE(partition.isValid());
}

TEST_F(ArcFlagTest, testMETIS)
{
  METISPartition partition(graph, 10);

  ASSERT_TRUE(partition.isValid());

  ArcFlagPreprocessor preprocessor(graph, costs, partition);
}
*/

class TestRouter : public Dijkstra
{
private:
  const ArcFlags& incomingFlags;
public:
  TestRouter(const Graph& graph,
             const ArcFlags& incomingFlags)
    : Dijkstra(graph),
      incomingFlags(incomingFlags)
  {}

  using Dijkstra::shortestPath;

  SearchResult shortestPath(Vertex source,
                            Vertex target,
                            const EdgeFunc<num>& costs) override
  {
    return shortestPath(source,
                        target,
                        costs,
                        incomingFlags.getFilter(target),
                        inf);
  }
};

TEST_F(ArcFlagTest, testCentralized)
{
  CentralizedPreprocessor centralizedPreprocessor(graph,
                                                  costs,
                                                  partition);

  //ArcFlagRouter router = centralizedPreprocessor.getRouter();

  TestRouter router(graph,
                    centralizedPreprocessor.getIncomingFlags());

  Dijkstra dijkstra(graph);

  const Region& region = *(partition.getRegions().begin());
  const std::vector<Vertex>& vertices = region.getVertices();

  for(const Vertex& source : vertices)
  {
    for(const Vertex& target : vertices)
    {
      auto result = router.shortestPath(source, target, costs);

      auto checkResult = dijkstra.shortestPath(source, target, costs);

      ASSERT_EQ(result.found, checkResult.found);

      if(result.found)
      {
        ASSERT_EQ(result.path.cost(costs), checkResult.path.cost(costs));
      }
    }
  }

  for(const Vertex& target : vertices)
  {
    bool boundary = false;
    for(const Edge& edge : graph.getAdjacentEdges(target))
    {
      if(partition.getRegion(edge.getOpposite(target)) != region)
      {
        boundary = true;
        break;
      }
    }

    if(!boundary)
    {
      continue;
    }

    for(const Vertex& source : graph.getVertices())
    {
      if(std::find(vertices.begin(),
                   vertices.end(),
                   source) != vertices.end())
      {
        continue;
      }

      auto result = router.shortestPath(source, target, costs);

      auto checkResult = dijkstra.shortestPath(source, target, costs);

      ASSERT_EQ(result.found, checkResult.found);

      if(result.found)
      {
        ASSERT_EQ(result.path.cost(costs), checkResult.path.cost(costs));
      }
    }
  }

  testRouter(router);

  auto arcFlagRouter = centralizedPreprocessor.getRouter();

  testRouter(arcFlagRouter);
}

/*
TEST_F(ArcFlagTest, testRouter)
{
  ArcFlagRouter router = preprocessor.getRouter();

  testRouter(router);
}
*/

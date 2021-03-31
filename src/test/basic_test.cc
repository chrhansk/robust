#include "basic_test.hh"

#include <algorithm>
#include <fstream>
#include <random>

#include "log.hh"
#include "reader/graph_reader.hh"

const int size = 10;

AbstractTest::AbstractTest()
{
  logInit();

  std::string directory = BASE_DIRECTORY;

  std::ifstream input(directory + "/" + INSTANCE + ".pbf");

  ReadResult result = GraphReader().readGraph(input);

  graph = result.graph;
  costMap = result.costs;
  deviationMap = result.deviations;
  points = result.points;
}

BasicTest::BasicTest()
  : AbstractTest(),
    costs(costMap.getValues()),
    deviations(deviationMap.getValues()),
    deviationSize(5)
{
  std::vector<Vertex> vertices = graph.getVertices().collect();

  auto engine = std::default_random_engine(18);
  std::uniform_int_distribution<int> distribution(0, vertices.size());

  std::random_shuffle(vertices.begin(), vertices.end(),
                      [&distribution, &engine](int i) -> int
                      {
                        return distribution(engine) % i;
                      });

  auto middle = vertices.begin();
  std::advance(middle, size);

  sources = std::vector<Vertex>(vertices.begin(), middle);

  auto rmiddle = vertices.rbegin();
  std::advance(rmiddle, size);

  targets = std::vector<Vertex>(vertices.rbegin(), rmiddle);
}


BasicRouterTest::BasicRouterTest()
{
  for(const Vertex& source : sources)
  {
    Dijkstra dijkstra(graph);
    for(const Vertex& target : targets)
    {
      auto result = dijkstra.shortestPath(source, target, costs);

      if(result.found)
      {
        results.push_back(Result(source, target, result.path.cost(costs)));
      }
    }
  }
}

void BasicRouterTest::testRouter(Router& router) const
{
  for(const Result& expected : results)
  {
    auto actual = router.shortestPath(expected.source, expected.target, costs);

    ASSERT_TRUE(actual.found);
    ASSERT_EQ(expected.cost, actual.path.cost(costs));
  }
}

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_map>

#include <gtest/gtest.h>

#include "arcflags/metis_partition.hh"
#include "contraction/nested_dissection_order.hh"
#include "robust/contraction/robust_contraction_preprocessor.hh"
#include "robust/contraction/parallel_robust_contraction_preprocessor.hh"

#include "basic_test.hh"

const int deviationSize = 5;

class RobustContractionTest : public BasicTest
{
public:
  RobustContractionTest() {}
};

/*
  TEST_F(RobustContractionTest, testContraction)
  {
  NestedDissectionOrder order(graph);

  Graph simpleGraph = order.simpleUndirected(graph);

  NestedDissectionOrder simpleOrder(simpleGraph);

  std::vector<Vertex> contractionOrder = simpleGraph.getVertices();

  std::sort(contractionOrder.begin(),
  contractionOrder.end(),
  VertexCompare<num>(order.getValues()));

  RobustContractionPreprocessor preprocessor(simpleGraph,
  costs,
  deviations,
  contractionOrder);
  }
*/

TEST(NestedDissectionOrder, testValidOrder)
{
  std::vector<Vertex> sources, targets;
  int n = 5;

  for(int i = 0; i < n; ++i)
  {
    sources.push_back(Vertex(i));
    targets.push_back(Vertex(n + i + 1));
  }

  std::vector<Vertex> vertices;
  vertices.insert(std::end(vertices), std::begin(sources), std::end(sources));
  Vertex middle(n);
  vertices.push_back(middle);
  vertices.insert(std::end(vertices), std::begin(targets), std::end(targets));

  std::vector<Edge> edges;

  for(int i = 0; i < n; ++i)
  {
    edges.push_back(Edge(sources[i], middle, i));
  }

  for(int i = 0; i < n; ++i)
  {
    edges.push_back(Edge(middle, targets[i], n + i));
  }

  Graph graph(vertices.size(), edges);

  NestedDissectionOrder order(graph);

  // the middle vertex is topologically the most important one, so it
  // should be last in the order.

  std::vector<Vertex> sorted = graph.getVertices().collect();

  std::sort(sorted.begin(),
            sorted.end(),
            VertexCompare<num>(order.getValues()));

  ASSERT_EQ(middle, *sorted.rbegin());
}

class ContractionTest : public testing::Test
{
protected:
  Vertex source, target;
  Edge upper, lower;
  Graph graph;
  EdgeMap<num> costs, deviations;
  EdgeValueMap<num> costValues, deviationValues;
public:
  ContractionTest();
};

ContractionTest::ContractionTest()
  : source(0),
    target(1),
    upper(source, target, 0),
    lower(source, target, 1),
    graph(2, {upper, lower}),
    costs(graph, 0),
    deviations(graph, 0),
    costValues(costs.getValues()),
    deviationValues(deviations.getValues())
{
  costs(lower) = 1;
  deviations(lower) = 1;
  deviations(upper) = 3;
}


TEST_F(ContractionTest, testTightenEdges)
{
  ParallelRobustContractionPreprocessor preprocessor(graph, costValues, deviationValues);

  preprocessor.computeHierarchy();
}

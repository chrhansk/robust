#include <gtest/gtest.h>

#include "graph/graph.hh"
#include "router/router.hh"

#include "contraction/contraction_hierarchy.hh"
#include "contraction/contraction_preprocessor.hh"
#include "contraction/parallel_contraction_preprocessor.hh"
#include "contraction/nested_dissection_order.hh"

#include "basic_test.hh"

class ContractionTest : public BasicRouterTest
{
};

TEST_F(ContractionTest, testContraction)
{
  ContractionPreprocessor preprocessor(graph, costs);
  ContractionHierarchy hierarchy(preprocessor.computeHierarchy());

  auto router = hierarchy.getRouter();

  testRouter(router);
}

TEST_F(ContractionTest, testParallelContraction)
{
  ParallelContractionPreprocessor preprocessor(graph, costs);
  ContractionHierarchy hierarchy(preprocessor.computeHierarchy());

  auto router = hierarchy.getRouter();

  testRouter(router);
}

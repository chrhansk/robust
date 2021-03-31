#ifndef ROBUST_ROUTER_TEST_HH
#define ROBUST_ROUTER_TEST_HH

#include "basic_test.hh"

#include "graph/vertex_map.hh"
#include "robust/robust_router.hh"

class RobustRouterTest : public BasicTest
{
protected:
  void testRobustRouter(RobustRouter& router) const;

  VertexMap<VertexMap<num>> values;

public:
  RobustRouterTest();
};

#define ADD_ROBUST_ROUTER_TEST(name, router_init)              \
                                                               \
  TEST_F(RobustRouterTest, Test_##name)                        \
  {                                                            \
    SimpleThetaRouter thetaRouter(graph,                       \
                                  costs,                       \
                                  deviations,                  \
                                  deviationSize);              \
                                                               \
    auto router = (router_init);                               \
    testRobustRouter(router);                                  \
  }

#endif /* ROBUST_ROUTER_TEST_HH */

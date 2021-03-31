#include "robust/robust_router_test.hh"

#include "arcflags/metis_partition.hh"

#include "robust/active/arcflag_active_router.hh"

#include "robust/arcflags/robust_arcflag_preprocessor.hh"
#include "robust/arcflags/simple_arcflags.hh"

TEST_F(RobustRouterTest, testArcFlagActiveRouter)
{
  METISPartition partition(graph, 64);

  RobustArcFlagPreprocessor preprocessor(graph,
                                         costs,
                                         deviations,
                                         partition);

  Bidirected<SimpleArcFlags> flags(graph, partition);

  preprocessor.computeFlags(flags, false);

  ArcFlagActiveRouter robustRouter(graph,
                                   costs,
                                   deviations,
                                   deviationSize,
                                   partition,
                                   flags);

  testRobustRouter(robustRouter);
}

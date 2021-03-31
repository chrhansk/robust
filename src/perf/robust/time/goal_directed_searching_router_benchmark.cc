#include "robust/searching_robust_router.hh"
#include "robust/theta/goal_directed_router.hh"

#include "robust/time/robust_benchmark.hh"

COMBINED_BENCHMARK(SearchingRobustRouter,
                   GoalDirectedRouter,
                   "GoalDirectedSearchingRouter")

#include "robust/searching_robust_router.hh"
#include "robust/theta/simple_theta_router.hh"

#include "robust/time/robust_benchmark.hh"

COMBINED_BENCHMARK(SearchingRobustRouter,
                   SimpleThetaRouter,
                   "SimpleSearchingRouter")

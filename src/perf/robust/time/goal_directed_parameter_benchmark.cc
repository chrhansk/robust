#include "basic_fixture.hh"

#include "robust/theta/bidirectional_goal_directed_router.hh"
#include "robust/time/simple_robust_router.hh"

const idx parameterCount = 100;
const idx deviationSize = 5;

BENCHMARK_DEFINE_F(BasicFixture, BM_alternating_parameter)
  (benchmark::State& state)
{
  float parameter = state.range(0) / ((float) parameterCount);

  BidirectionalGoalDirectedRouter thetaRouter(graph,
                                              costs,
                                              deviations);

  thetaRouter.setRecomputationFactor(parameter);

  SimpleRobustRouter router(graph,
                            costs,
                            deviations,
                            deviationSize,
                            thetaRouter,
                            false);

  while(state.KeepRunning())
  {
    for(const Vertex& source : sources)
    {
      for(const Vertex& target : targets)
      {
        router.shortestPath(source, target);
      }
    }
  }

  state.SetLabel(std::to_string(parameter));

}

BENCHMARK_REGISTER_F(BasicFixture, BM_alternating_parameter)
->DenseRange(1, parameterCount - 1);

BENCHMARK_MAIN();

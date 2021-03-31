#ifndef TIMER_HH
#define TIMER_HH

#include <algorithm>
#include <numeric>
#include <chrono>

#include "log.hh"

class Timer
{
private:
  std::chrono::time_point<std::chrono::steady_clock> start;

public:
  Timer() :
    start(std::chrono::steady_clock::now())
  {}

  double elapsed()
  {
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count();
  }
};

struct Result
{
  double totalSeconds;
  int totalIterations;

  double averageSeconds() const
  {
    return totalSeconds / (double(totalIterations));
  }
};

template<class Func>
class FunctionTimer
{
private:
  int minIterations;
  double minSeconds;
  Func func;

public:
  FunctionTimer(int minIterations,
                double minSeconds,
                Func func)
    : minIterations(minIterations),
      minSeconds(minSeconds),
      func(func)
  {}

  Result execute()
  {
    int totalIterations = 0;
    double totalSeconds = 0.;

    while(true)
    {
      Timer timer;

      func();

      totalSeconds += timer.elapsed();
      totalIterations++;

      if(minIterations != -1 && totalIterations >= minIterations)
      {
        break;
      }

      if(minSeconds != -1 && totalSeconds >= minSeconds)
      {
        break;
      }
    }

    Log(info) << "Executed " << totalIterations
              << " iterations in " << totalSeconds << " seconds";

    return Result{totalSeconds, totalIterations};
  }
};

struct RangedResult
{
  std::vector<Result> results;

  double averageSeconds() const
  {
    assert(!results.empty());

    double average = 0;

    for(const auto & result : results)
    {
      average += result.averageSeconds();
    }

    return average / (results.size());
  }

  double minSeconds() const
  {
    assert(!results.empty());

    double minSecs = inf;

    for(const auto & result : results)
    {
      minSecs = std::min(minSecs, result.averageSeconds());
    }

    return minSecs;
  }

  double maxSeconds() const
  {
    assert(!results.empty());

    double maxSecs = -inf;

    for(const auto & result : results)
    {
      maxSecs = std::max(maxSecs, result.averageSeconds());
    }

    return maxSecs;
  }
};

template<class It, class Func>
class RangedBenchmark
{
private:
  It begin, end;
  Func func;

  int minIterations;
  double minSeconds;

public:
  RangedBenchmark(It begin,
                  It end,
                  int minIterations,
                  double minSeconds,
                  Func func)
    : begin(begin),
      end(end),
      minIterations(minIterations),
      minSeconds(minSeconds),
      func(func)
  {}

  RangedResult execute()
  {
    double executionMinSeconds = std::numeric_limits<double>::max();
    double executionMaxSeconds = std::numeric_limits<double>::min();
    double executionAverageSeconds = 0;
    int numValues = std::distance(begin, end);

    std::vector<Result> results;

    for(It it = begin; it < end; ++it)
    {
      FunctionTimer<std::function<void()>> functionTimer(
        minIterations,
        minSeconds,
        [&]() {
          func(*it);
        });

      auto result = functionTimer.execute();

      executionMinSeconds = std::min(executionMinSeconds, result.averageSeconds());
      executionMaxSeconds = std::max(executionMaxSeconds, result.averageSeconds());
      executionAverageSeconds += result.averageSeconds();

      results.push_back(result);
    }

    executionAverageSeconds /= numValues;

    Log(info) << "Executed a range of " << numValues
              << " iterations, min = " << executionMinSeconds
              << " , max = " << executionMaxSeconds
              << " , avg = " << executionAverageSeconds;

    return RangedResult{results};
  }
};

#endif /* TIMER_HH */

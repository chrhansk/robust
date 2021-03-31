#ifndef ROBUST_SCORE_FUNCTION_HH
#define ROBUST_SCORE_FUNCTION_HH

#include "graph/graph.hh"

#include "robust_contraction_pair.hh"

class RobustScoreFunction
{
protected:
  const Graph& graph;
  const EdgeFunc<const ContractionRange&>& contractionRanges;

public:
  RobustScoreFunction(const Graph& graph,
                      const EdgeFunc<const ContractionRange&>& contractionRanges)
    : graph(graph),
      contractionRanges(contractionRanges)
  {}

  virtual float getScore(Vertex vertex,
                         const std::vector<RobustContractionPair>& pairs) = 0;
};

template <class... Ts>
class SumFunction
{
public:
  float getScore(Vertex vertex,
                 const std::vector<RobustContractionPair>& pairs)
  {
    return 0;
  }
};

template <class T, class... Ts>
class SumFunction<T, Ts...> : SumFunction<Ts...>
{
private:
  T tail;

public:
  SumFunction(T t, Ts... ts) : SumFunction<Ts...>(ts...), tail(t) {}

  float getScore(Vertex vertex,
                 const std::vector<RobustContractionPair>& pairs)
  {
    return SumFunction<Ts...>::getScore(vertex, pairs) + tail.getScore(vertex, pairs);
  }
};


#endif /* ROBUST_SCORE_FUNCTION_HH */

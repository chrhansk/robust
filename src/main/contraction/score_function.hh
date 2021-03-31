#ifndef SCORE_FUNCTION_HH
#define SCORE_FUNCTION_HH

#include "util.hh"
#include "graph/graph.hh"
#include "witness_path_search.hh"

#include "contraction_result.hh"

class ScoreFunction
{
public:
  virtual float getScore(Vertex vertex,
                         const std::vector<ContractionPair>& pairs) = 0;

  virtual void vertexContracted(Vertex vertex,
                                const std::vector<ContractionResult>& results)
  {}
};

template <class... Ts>
class SumFunction
{
public:
  float getScore(Vertex vertex,
                 const std::vector<ContractionPair>& pairs)
  {
    return 0;
  }

  void vertexContracted(Vertex vertex,
                        const std::vector<ContractionResult>& results)
  {}

};

template <class T, class... Ts>
class SumFunction<T, Ts...> : SumFunction<Ts...>
{
private:
  T tail;

public:
  SumFunction(T t, Ts... ts) : SumFunction<Ts...>(ts...), tail(t) {}

  float getScore(Vertex vertex,
                 const std::vector<ContractionPair>& pairs)
  {
    return SumFunction<Ts...>::getScore(vertex, pairs) + tail.getScore(vertex, pairs);
  }

  void vertexContracted(Vertex vertex,
                        const std::vector<ContractionResult>& results)
  {
    SumFunction<Ts...>::vertexContracted(vertex, results);
    tail.vertexContracted(vertex, results);
  }
};

#endif /* SCORE_FUNCTION_HH */

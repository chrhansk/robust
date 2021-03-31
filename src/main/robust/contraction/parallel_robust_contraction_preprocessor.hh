#ifndef PARALLEL_ROBUST_CONTRACTION_PREPROCESSOR_HH
#define PARALLEL_ROBUST_CONTRACTION_PREPROCESSOR_HH

#include "abstract_robust_contraction_preprocessor.hh"

class ParallelRobustContractionPreprocessor
  : public AbstractRobustContractionPreprocessor
{
private:
  idx neighborhoodSize;

  class ScoreComparator
  {
  private:
    const VertexMap<float>& scores;
  public:
    ScoreComparator(const VertexMap<float>& scores)
      : scores(scores)
    {}

    bool operator()(const Vertex& first, const Vertex& second) const
    {
      float left = scores(first);
      float right = scores(second);

      if(left < right)
      {
        return true;
      }
      else if(left > right)
      {
        return false;
      }

      return first < second;
    }
  };

  bool isMinimal(const Graph& graph,
                 const Vertex& vertex,
                 idx neighborhoodSize,
                 const Contracted& contracted,
                 const ScoreComparator& comparator) const;

public:
  ParallelRobustContractionPreprocessor(const Graph& graph,
                                        const EdgeFunc<num>& costs,
                                        const EdgeFunc<num>& deviations,
                                        idx neighborhoodSize = 2)
    : AbstractRobustContractionPreprocessor(graph, costs, deviations),
      neighborhoodSize(neighborhoodSize)
  {}

  RobustContractionHierarchy computeHierarchy() const override;
};


#endif /* PARALLEL_ROBUST_CONTRACTION_PREPROCESSOR_HH */

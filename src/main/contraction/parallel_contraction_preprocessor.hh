#ifndef PARALLEL_CONTRACTION_PREPROCESSOR_HH
#define PARALLEL_CONTRACTION_PREPROCESSOR_HH

#include "abstract_contraction_preprocessor.hh"

class ParallelContractionPreprocessor : AbstractContractionPreprocessor
{
private:
  const idx neighborhoodSize;

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
                 const ScoreComparator& comparator);

public:
  ParallelContractionPreprocessor(const Graph& graph,
                                  const EdgeFunc<num>& costs,
                                  idx neighborhoodSize = 2);

  ContractionHierarchy computeHierarchy() override;
};


#endif /* PARALLEL_CONTRACTION_PREPROCESSOR_HH */

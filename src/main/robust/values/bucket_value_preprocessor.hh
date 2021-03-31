#ifndef BUCKET_VALUE_PREPROCESSOR_HH
#define BUCKET_VALUE_PREPROCESSOR_HH

#include "abstract_value_preprocessor.hh"

class BucketValuePreprocessor : public AbstractValuePreprocessor
{
private:
  class Bucket
  {
  private:
    std::vector<Vertex> vertices;

  public:
    std::vector<Vertex>& getVertices()
    {
      return vertices;
    }

    const std::vector<Vertex>& getVertices() const
    {
      return vertices;
    }
  };

  std::pair<std::vector<Bucket>, std::vector<Bucket>>
  medianPartition(const Boundary& sourceBoundary,
                  const Boundary& targetBoundary) const;

  std::pair<std::vector<Bucket>, std::vector<Bucket>>
  meanPartition(const Boundary& sourceBoundary,
                const Boundary& targetBoundary) const;

  idx numBuckets;

public:
  BucketValuePreprocessor(const Graph& graph,
                          const EdgeFunc<num>& costs,
                          const EdgeFunc<num>& deviations,
                          num deviationSize,
                          const Partition& partition,
                          idx numBuckets = 10)
    : AbstractValuePreprocessor(graph,
                                costs,
                                deviations,
                                deviationSize,
                                partition),
      numBuckets(numBuckets)
  {}


  ValueSet requiredValues(const Region& sourceRegion,
                          const Region& targetRegion,
                          const ValueSet& possibleValues) const override;

  using AbstractValuePreprocessor::requiredValues;
};


#endif /* BUCKET_VALUE_PREPROCESSOR_HH */

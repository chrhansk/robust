#ifndef ABSTRACT_VALUE_PREPROCESSOR_HH
#define ABSTRACT_VALUE_PREPROCESSOR_HH

#include <unordered_map>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "arcflags/partition.hh"

#include "router/label.hh"

#include "robust/robust_utils.hh"

#include "vertex_pair_map.hh"

class AbstractValuePreprocessor
{
protected:

  class RootedLabel : public AbstractLabel
  {
  private:
    Vertex root;
  public:
    RootedLabel()
      : AbstractLabel()
    {}

    RootedLabel(Vertex vertex, num cost, Vertex root)
      : AbstractLabel(vertex, cost),
        root(root)
    {}

    Vertex getRoot() const
    {
      return root;
    }
  };

  class Bound
  {
  private:
    num min, max;

  public:
    Bound(num min, num max)
      : min(min), max(max)
    {}

    Bound()
      : min(inf), max(0)
    {}

    num getMin() const
    {
      return min;
    }

    num getMax() const
    {
      return max;
    }

    void update(const num& value)
    {
      if(value < getMin())
      {
        setMin(value);
      }
      if(value > getMax())
      {
        setMax(value);
      }
    }

    void setMin(const num& value)
    {
      min = value;
    }

    void setMax(const num& value)
    {
      max = value;
    }

    bool operator<=(const Bound& other) const
    {
      return getMax() <= other.getMin();
    }

  };

  class BoundValues : public Bound
  {
  private:
    ValueVector values;

  public:
    BoundValues()
    {}

    const ValueVector& getValues() const
    {
      return values;
    }

    ValueVector& getValues()
    {
      return values;
    }

    void update(const Bound& bound, num value)
    {
      if(bound <= (*this))
      {
        setMin(bound.getMin());
        setMax(bound.getMax());
        getValues() = {value};
      }
      else if((*this) <= bound)
      {
      }
      else
      {
        setMin(std::min(getMin(),
                        bound.getMin()));

        setMax(std::max(getMax(),
                        bound.getMax()));

        getValues().push_back(value);
      }
    }
  };

  const Graph& graph;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  const num deviationSize;
  const Partition& partition;
  const ValueVector values;

  DistanceMap findShortestPaths(const Region& sourceRegion,
                                const Region& targetRegion) const;

  VertexPairMap<num> findDistances(const std::vector<Vertex>& sources,
                                   const std::vector<Vertex>& targets,
                                   const EdgeFunc<num>& costs) const;

public:
  AbstractValuePreprocessor(const Graph& graph,
                            const EdgeFunc<num>& costs,
                            const EdgeFunc<num>& deviations,
                            num deviationSize,
                            const Partition& partition)
    : graph(graph),
      costs(costs),
      deviations(deviations),
      deviationSize(deviationSize),
      partition(partition),
      values(thetaValues(graph, deviations))
  {}

  virtual ValueSet requiredValues(const Region& sourceRegion,
                                  const Region& targetRegion,
                                  const ValueSet& possibleValues) const = 0;

  virtual ValueSet requiredValues(const Region& sourceRegion,
                                  const Region& targetRegion) const;
};


#endif /* ABSTRACT_VALUE_PREPROCESSOR_HH */

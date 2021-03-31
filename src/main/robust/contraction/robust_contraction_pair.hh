#ifndef ROBUST_CONTRACTION_PAIR_HH
#define ROBUST_CONTRACTION_PAIR_HH

#include <cassert>
#include <vector>

#include "graph/graph.hh"
#include "graph/vertex_map.hh"

#include "path/path.hh"

#include "robust/robust_utils.hh"

#include "contraction_range.hh"

class RobustContractionPreprocessor;

class RobustContractionPair
{
private:
  Vertex source;
  Vertex target;
  Path defaultPath;
  std::vector<num>::const_iterator begin, end;

public:
  RobustContractionPair(Vertex source,
                        Vertex target,
                        const Path& defaultPath,
                        std::vector<num>::const_iterator begin,
                        std::vector<num>::const_iterator end);

  Vertex getSource() const
  {
    return source;
  }

  Vertex getTarget() const
  {
    return target;
  }

  const Path& getDefaultPath() const
  {
    return defaultPath;
  }

  const ValueIterator& getBegin() const
  {
    return begin;
  }

  const ValueIterator& getEnd() const
  {
    return end;
  }

  int valueCount() const
  {
    return std::distance(getBegin(), getEnd());
  }

  num getMinimum() const
  {
    ReverseValueIterator revBegin(getEnd());
    return *revBegin;
  }

  num getMaximum() const
  {
    return *getBegin();
  }

  bool check(const Graph& graph,
             const ValueVector& values,
             const VertexFunc<bool>& contracted,
             const EdgeFunc<const ContractionRange&>& contractionRanges) const;
};


#endif /* ROBUST_CONTRACTION_PAIR_HH */

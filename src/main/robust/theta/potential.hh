#ifndef POTENTIAL_HH
#define POTENTIAL_HH

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

template <Direction direction>
class PartialDistanceMap : public VertexFunc<num>
{
private:
  const Graph& graph;
  num defaultValue;
  VertexMap<num> values;
public:
  PartialDistanceMap(const Graph& graph)
    : graph(graph),
      defaultValue(0),
      values(graph, inf)
  {
  }

  void setDefaultValue(num value)
  {
    defaultValue = value;
  }

  void reset()
  {
    defaultValue = inf;
    values.reset(inf);
  }

  num operator()(const Vertex& vertex) const override
  {
    assert(defaultValue < inf);

    if(direction == Direction::INCOMING)
    {
      return std::min(defaultValue, values(vertex));
    }
    else
    {
      return -1 * std::min(defaultValue, values(vertex));
    }
  }

  void setValue(const Vertex& vertex, num value)
  {
    values(vertex) = value;
  }

};


class Potential : public VertexFunc<num>
{
private:
  const Graph& graph;
public:
  Potential(const Graph& graph)
    : graph(graph)
  {}

  bool isValidFor(const EdgeFunc<num>& costs) const;
  bool isTightFor(const EdgeFunc<num>& costs,
                  const Edge& edge) const;

  num potentialPathCost(num actualPathCost,
                        Vertex source,
                        Vertex target) const
  {
    return actualPathCost - (*this)(source) + (*this)(target);
  }

  num actualPathCost(num potentialPathCost,
                     Vertex source,
                     Vertex target) const
  {
    return potentialPathCost + (*this)(source) - (*this)(target);
  }

};

class SimplePotential : public Potential
{
private:
  const VertexFunc<num>& values;

public:
  SimplePotential(const Graph& graph,
                  const VertexFunc<num>& values)
    : Potential(graph),
      values(values)
  {}

  num operator()(const Vertex& vertex) const override
  {
    return values(vertex);
  }

};

class PotentialCosts : public EdgeFunc<num>
{
private:
  const EdgeFunc<num>& costs;
  const Potential& potential;

public:
  PotentialCosts(const EdgeFunc<num>& costs,
                 const Potential& potential)
    : costs(costs),
      potential(potential)
  {}

  num operator()(const Edge& edge) const override
  {
    return costs(edge)
      - potential(edge.getSource())
      + potential(edge.getTarget());
  }
};

#endif /* POTENTIAL_HH */

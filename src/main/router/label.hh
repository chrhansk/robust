#ifndef LABEL_HH
#define LABEL_HH

#include "graph/graph.hh"

enum class State
{
  UNKNOWN, LABELED, SETTLED
};

class AbstractLabel
{
private:
  Vertex vertex;
  num cost;
  State state;

public:
  AbstractLabel()
    : cost(inf),
      state(State::UNKNOWN)
  {}
  AbstractLabel(Vertex vertex, num cost)
    : vertex(vertex),
      cost(cost),
      state(State::LABELED)
  {}

  num getCost() const
  {
    return cost;
  }

  bool operator<(const AbstractLabel& other) const
  {
    return cost < other.cost;
  }

  bool operator>(const AbstractLabel& other) const
  {
    return cost > other.cost;
  }

  Vertex getVertex() const
  {
    return vertex;
  }

  State getState() const
  {
    return state;
  }

  void setState(State state)
  {
    this->state = state;
  }

};

class SimpleLabel : public AbstractLabel
{
public:
  SimpleLabel(Vertex vertex, num cost)
    : AbstractLabel(vertex, cost) {}
  SimpleLabel() {}
};

class Label : public AbstractLabel
{
private:
  Edge edge;

public:
  Label(Vertex vertex, const Edge& edge, num cost)
    : AbstractLabel(vertex, cost),
      edge(edge)
  {}

  Label() {}

  Edge getEdge() const
  {
    return edge;
  }
};

#endif /* LABEL_HH */

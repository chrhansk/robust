#include <vector>

#include <gtest/gtest.h>

#include "graph/graph.hh"
#include "graph/edge_map.hh"
#include "graph/vertex_map.hh"

#include "router/router.hh"

const std::string INSTANCE("potsdam");

class AbstractTest : public testing::Test
{
protected:
  Graph graph;
  EdgeMap<num> costMap;
  EdgeMap<num> deviationMap;
  VertexMap<Point> points;

public:
  AbstractTest();
};

class BasicTest : public AbstractTest
{
public:
  BasicTest();

protected:
  EdgeValueMap<num> costs;
  EdgeValueMap<num> deviations;
  idx deviationSize;

  std::vector<Vertex> sources, targets;
};

class BasicRouterTest : public BasicTest
{
private:
  class Result
  {
  public:
    Result(Vertex source, Vertex target, num cost)
      : source(source),
        target(target),
        cost(cost)
    {}
    Vertex source, target;
    num cost;
  };
  std::vector<Result> results;
public:
  BasicRouterTest();

  void testRouter(Router& router) const;
};

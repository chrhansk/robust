#ifndef THETA_TREE_HH
#define THETA_TREE_HH

#include "graph/vertex_map.hh"

#include "reduced_costs.hh"
#include "robust_utils.hh"

/**
 * A shortest path tree with respect to ReducedCosts.
 * Initially the tree is a shortest Path tree with
 * respect to the ReducedCosts given by \f$ \theta = 0 \f$.
 * However, it is possible to reset the tree to another greater
 * value. In this case the tree is not rebuilt from scratch.
 * Instead, only the changed parts of the tree are recomputed.
 * This does in general take much less time
 * than an entire recomputation.
 **/
class ThetaTree
{
  class Node
  {
  private:
    std::vector<Edge> children;
    Edge parent;
    num distance;

  public:
    Node()
      : distance(0)
    {}

    const std::vector<Edge>& getChildren() const
    {
      return children;
    }

    std::vector<Edge>& getChildren()
    {
      return children;
    }

    num getDistance() const
    {
      return distance;
    }

    void setDistance(num value)
    {
      distance = value;
    }

    const Edge& getParent() const
    {
      return parent;
    }

    void setParent(const Edge& value)
    {
      parent = value;
    }

    void removeChild(const Edge& edge)
    {
      std::vector<Edge>& children = getChildren();

      assert(std::find(children.begin(), children.end(), edge) != children.end());

      children.erase(std::remove(children.begin(), children.end(), edge),
                     children.end());
    }

    void setParentNode(Node& parentNode,
                       const Edge& edge)
    {
      assert(parent.getTarget() == edge.getTarget());
      setParent(edge);
      parentNode.getChildren().push_back(edge);
    }
  };

  const Graph& graph;
  Vertex root;
  const EdgeFunc<num>& costs;
  const EdgeFunc<num>& deviations;
  num value;

  VertexMap<Node> nodes;

public:
  ThetaTree(const Graph& graph,
             const Vertex& root,
             const EdgeFunc<num>& costs,
             const EdgeFunc<num>& deviations)
    : graph(graph),
      root(root),
      costs(costs),
      deviations(deviations),
      value(0),
      nodes(graph, Node())
  {
    recomputeTree();
  }

  Vertex getRoot() const
  {
    return root;
  }

  num getValue() const
  {
    return value;
  }

  num getDistance(const Vertex& vertex) const
  {
    return nodes(vertex).getDistance();
  }

  void reset(num newValue);

private:
  void recomputeTree();

  bool check() const;

  bool checkDistances() const;
};


#endif /* THETA_TREE_HH */

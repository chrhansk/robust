#include "theta_tree.hh"

#include <queue>

#include "graph/vertex_set.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

void ThetaTree::recomputeTree()
{
  LabelHeap<Label> heap(graph);

  heap.update(Label(root, Edge(), 0));

  ReducedCosts reducedCosts(costs, deviations, value);

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();
    Node& currentNode = nodes(current.getVertex());

    if(current.getVertex() != root)
    {
      const Edge& edge = current.getEdge();
      Node& parentNode = nodes(edge.getSource());

      currentNode.setDistance(current.getCost());
      currentNode.setParent(edge);
      currentNode.setParentNode(parentNode, edge);
    }

    for(const Edge& edge : graph.getOutgoing(current.getVertex()))
    {
      Label nextLabel = Label(edge.getTarget(),
                              edge, current.getCost() + reducedCosts(edge));

      heap.update(nextLabel);
    }
  }

  assert(check());
}

void ThetaTree::reset(num newValue)
{
  if(newValue == value)
  {
    return;
  }

  assert(newValue > value);

  value = newValue;

  ReducedCosts reducedCosts(costs, deviations, value);

  std::queue<Vertex> queue;
  queue.push(root);


  while(!queue.empty())
  {
    Vertex current = queue.front();
    queue.pop();

    for(const Edge& edge : nodes(current).getChildren())
    {
      Vertex other = edge.getTarget();

      nodes(other).setDistance(nodes(current).getDistance() + reducedCosts(edge));

      queue.push(other);
    }
  }

  assert(checkDistances());

  queue.push(root);
  VertexSet invalid(graph);

  LabelHeap<Label> heap(graph);

  while(!queue.empty())
  {
    Vertex current = queue.front();
    queue.pop();

    Node& currentNode = nodes(current);

    if(invalid.contains(current))
    {
      continue;
    }

    for(const Edge& edge : graph.getOutgoing(current))
    {
      const Vertex other = edge.getTarget();
      Node& otherNode = nodes(other);

      const num newDistance = currentNode.getDistance() + reducedCosts(edge);

      if(newDistance < otherNode.getDistance())
      {
        Node& oldParent = nodes(otherNode.getParent().getSource());

        oldParent.removeChild(otherNode.getParent());
        otherNode.setParentNode(currentNode, edge);
        otherNode.setDistance(newDistance);

        heap.update(Label(other, edge, newDistance));

        invalid.insert(other);
      }
    }

    for(const Edge& edge : currentNode.getChildren())
    {
      const Vertex other = edge.getTarget();

      queue.push(other);
    }
  }

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();
    const Vertex& currentVertex = current.getVertex();
    Node& currentNode = nodes(currentVertex);

    assert(currentNode.getDistance() == current.getCost());

    for(const Edge& edge : graph.getOutgoing(currentVertex))
    {
      Vertex other = edge.getTarget();

      Node& otherNode = nodes(other);

      num newDistance = currentNode.getDistance() + reducedCosts(edge);

      if(newDistance < otherNode.getDistance())
      {
        Label nextLabel = Label(other,
                                edge, current.getCost() + reducedCosts(edge));

        Node& oldParent = nodes(otherNode.getParent().getSource());
        oldParent.removeChild(otherNode.getParent());
        otherNode.setParentNode(currentNode, edge);
        otherNode.setDistance(newDistance);

        heap.update(nextLabel);
      }
    }
  }

  assert(check());
}

bool ThetaTree::check() const
{
  ReducedCosts reducedCosts(costs, deviations, value);

  std::queue<Vertex> queue;
  queue.push(root);


  while(!queue.empty())
  {
    Vertex current = queue.front();
    const Node& currentNode = nodes(current);

    queue.pop();

    for(const Edge& edge : currentNode.getChildren())
    {
      Vertex other = edge.getTarget();

      const Node& otherNode = nodes(other);

      if(otherNode.getParent().getSource() != current)
      {
        return false;
      }

      if(otherNode.getDistance() - currentNode.getDistance() != reducedCosts(edge))
      {
        return false;
      }

      queue.push(other);
    }
  }

  return true;
}

bool ThetaTree::checkDistances() const
{
  ReducedCosts reducedCosts(costs, deviations, value);

  std::queue<Vertex> queue;
  queue.push(root);


  while(!queue.empty())
  {
    Vertex current = queue.front();
    const Node& currentNode = nodes(current);

    queue.pop();

    for(const Edge& edge : currentNode.getChildren())
    {
      Vertex other = edge.getTarget();

      const Node& otherNode = nodes(other);

      if(otherNode.getDistance() - currentNode.getDistance() != reducedCosts(edge))
      {
        return false;
      }

      queue.push(other);
    }
  }

  return true;
}

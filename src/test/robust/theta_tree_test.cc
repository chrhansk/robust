#include "basic_test.hh"

#include "robust/theta_tree.hh"

class ThetaTreeTest : public BasicTest
{
public:
  ThetaTreeTest();

  void testTree(const ThetaTree& thetaTree, num value) const;

protected:
  Vertex root;
};

ThetaTreeTest::ThetaTreeTest()
  : root(*(graph.getVertices().begin()))
{
}

void ThetaTreeTest::testTree(const ThetaTree& thetaTree, num value) const
{
  ReducedCosts reducedCosts(costs, deviations, value);

  LabelHeap<Label> heap(graph);
  heap.update(Label(root, Edge(), 0));

  while(!heap.isEmpty())
  {
    const Label& current = heap.extractMin();

    for(const Edge& edge : graph.getOutgoing(current.getVertex()))
    {
      Label nextLabel = Label(edge.getTarget(),
                              edge, current.getCost() + reducedCosts(edge));

      heap.update(nextLabel);
    }

  }

  for(const Vertex& vertex : graph.getVertices())
  {
    ASSERT_EQ(thetaTree.getDistance(vertex), heap.getLabel(vertex).getCost());
  }
}


TEST_F(ThetaTreeTest, testInitialTree)
{
  ThetaTree thetaTree(graph, root, costs, deviations);

  num value = thetaTree.getValue();
  ReducedCosts reducedCosts(costs, deviations, value);

  testTree(thetaTree, value);
}

TEST_F(ThetaTreeTest, testRecomputation)
{
  ThetaTree thetaTree(graph, root, costs, deviations);

  const ValueVector values = thetaValues(graph, deviations);

  for(auto it = values.rbegin(); it < values.rend(); ++it)
  {
    const num value = *it;

    thetaTree.reset(value);

    testTree(thetaTree, value);
  }
}

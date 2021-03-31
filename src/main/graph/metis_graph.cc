#include "metis_graph.hh"

#include <cassert>
#include <cstdlib>

static Graph simpleUndirected(const Graph& graph)
{
  std::vector<Edge> edges;
  int index = 0;

  for(const Edge& edge : graph.getEdges())
  {
    Vertex source = edge.getSource();
    Vertex target = edge.getTarget();
    if(source < target)
    {
      edges.push_back(Edge(source, target, index++));
    }
    else
    {
      bool hasMirror = false;

      for(const Edge& edge : graph.getOutgoing(target))
      {
        if(edge.getTarget() == source)
        {
          hasMirror = true;
          break;
        }
      }

      if(!hasMirror)
      {
        edges.push_back(Edge(source, target, index++));
      }
    }
  }

  return Graph(graph.getVertices().size(), edges);
}


METISGraph::METISGraph(const Graph& graph)
{
  Graph simpleGraph = simpleUndirected(graph);

  numVertices = simpleGraph.getVertices().size();
  numEdges = simpleGraph.getEdges().size();

  xadj = (METIS::idx_t*) malloc(sizeof(METIS::idx_t) * (numVertices + 1));
  adjncy = (METIS::idx_t*) malloc(sizeof(METIS::idx_t) * (2*numEdges));

  int i = 0, j = 0;

  xadj[0] = 0;

  for(const Vertex& vertex : simpleGraph.getVertices())
  {
    for(const Edge& edge : simpleGraph.getOutgoing(vertex))
    {
      adjncy[j++] = edge.getTarget().getIndex();
    }

    for(const Edge& edge : simpleGraph.getIncoming(vertex))
    {
      adjncy[j++] = edge.getSource().getIndex();
    }

    xadj[++i] = j;
  }

  for(i = 0; i < numVertices; ++i)
  {
    assert(xadj[i] <= xadj[i + 1]);
  }

  for(i = 0; i < (numVertices + 1); ++i)
  {
    assert(xadj[i] >= 0 and xadj[i] <= (2*numEdges));
  }

  for(int j = 0; j < 2*numEdges; ++j)
  {
    assert(adjncy[j] >= 0 and adjncy[j] < numVertices);
  }

}

METISGraph::~METISGraph()
{
  free(xadj);
  free(adjncy);
}

METIS::idx_t* METISGraph::getXAdj() const
{
  return xadj;
}

METIS::idx_t* METISGraph::getAdjncy() const
{
  return adjncy;
}

METIS::idx_t METISGraph::getNumEdges() const
{
  return numEdges;
}

METIS::idx_t METISGraph::getNumVertices() const
{
  return numVertices;
}

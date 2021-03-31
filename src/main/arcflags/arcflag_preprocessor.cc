#include "arcflag_preprocessor.hh"

#include <unordered_set>

#include <tbb/tbb.h>

#include "log.hh"
#include "router/label.hh"
#include "router/label_heap.hh"

ArcFlagPreprocessor::ArcFlagPreprocessor(const Graph& graph,
                                         const EdgeFunc<num>& costs,
                                         const Partition& partition,
                                         bool parallel)
  : graph(graph), costs(costs), partition(partition),
    outgoingFlags(graph, partition),
    incomingFlags(graph, partition)
{
  int boundarySize = 0;

  for(const Edge& edge : graph.getEdges())
  {
    const Region& sourceRegion = partition.getRegion(edge.getSource());
    const Region& targetRegion = partition.getRegion(edge.getTarget());

    if(sourceRegion == targetRegion)
    {
      outgoingFlags.setFlag(edge, sourceRegion);
      incomingFlags.setFlag(edge, targetRegion);
    }
    else
    {
      ++boundarySize;
      overlappingEdges.push_back(edge);
    }
  }

  Log(info) << "Found " << boundarySize
             << " overlapping edges";

  if(parallel)
  {
    setFlagsParallel(Direction::OUTGOING);
    setFlagsParallel(Direction::INCOMING);
  }
  else
  {
    setFlags(Direction::OUTGOING);
    setFlags(Direction::INCOMING);
  }

  Log(info) << "Outgoing flags: " << outgoingFlags
             << ", incoming flags: " << incomingFlags;
}

ArcFlagPreprocessor::~ArcFlagPreprocessor()
{
}

void ArcFlagPreprocessor::setFlags(Direction direction)
{
  ArcFlags& arcFlags = (direction == Direction::OUTGOING) ?
    outgoingFlags : incomingFlags;

  std::unordered_set<Vertex> vertices;

  for(const Edge& edge : overlappingEdges)
  {
    vertices.insert(edge.getEndpoint(opposite(direction)));
  }

  Log(info) << "Computing flags for " << vertices.size()
             << " vertices";

  for(const Vertex& vertex : vertices)
  {
    const Region& vertexRegion = partition.getRegion(vertex);

    LabelHeap<Label> heap(graph);
    heap.update(Label(vertex, Edge(), 0));

    while(!heap.isEmpty())
    {
      const Label& current = heap.extractMin();

      if(current.getVertex() != vertex)
      {
        arcFlags.setFlag(current.getEdge(), vertexRegion);

        if(vertexRegion == partition.getRegion(current.getVertex()))
        {
          continue;
        }
      }

      for(const Edge& edge : graph.getEdges(current.getVertex(),
                                            direction))
      {
        Vertex nextVertex = edge.getEndpoint(direction);

        Label nextLabel = Label(nextVertex,
                                edge,
                                current.getCost() + costs(edge));

        heap.update(nextLabel);
      }
    }
  }
}
void ArcFlagPreprocessor::setFlagsParallel(Direction direction)
{
  ArcFlags& arcFlags = (direction == Direction::OUTGOING) ?
    outgoingFlags : incomingFlags;

  std::unordered_set<Vertex> vertexSet;

  for(const Edge& edge : overlappingEdges)
  {
    vertexSet.insert(edge.getEndpoint(opposite(direction)));
  }

  Log(info) << "Computing flags for " << vertexSet.size()
             << " vertices";

  std::vector<Vertex> vertices(vertexSet.begin(), vertexSet.end());

  tbb::spin_mutex mutex;

  tbb::parallel_do(vertices.begin(),
                   vertices.end(),
                   [this, &mutex, &arcFlags, direction](const Vertex& vertex)
                   {
                     const Region& vertexRegion = partition.getRegion(vertex);
                     std::vector<Edge> edges;

                     LabelHeap<Label> heap(graph);
                     heap.update(Label(vertex, Edge(), 0));

                     while(!heap.isEmpty())
                     {
                       const Label& current = heap.extractMin();

                       if(current.getVertex() != vertex)
                       {
                         edges.push_back(current.getEdge());

                         if(vertexRegion == partition.getRegion(current.getVertex()))
                         {
                           continue;
                         }
                       }

                       for(const Edge& edge : graph.getEdges(current.getVertex(),
                                                             direction))
                       {
                         Vertex nextVertex = edge.getEndpoint(direction);

                         Label nextLabel = Label(nextVertex,
                                                 edge,
                                                 current.getCost() + costs(edge));

                         heap.update(nextLabel);
                       }
                     }

                     {
                       tbb::spin_mutex::scoped_lock lock(mutex);

                       for(const Edge& edge : edges)
                       {
                         arcFlags.setFlag(edge, vertexRegion);
                       }

                     }

                   });
}

ArcFlagRouter ArcFlagPreprocessor::getRouter() const
{
  return ArcFlagRouter(graph,
                       outgoingFlags,
                       incomingFlags,
                       partition);
}

#include "centralized_preprocessor.hh"

#include <unordered_set>

#include <boost/heap/d_ary_heap.hpp>

#include "log.hh"

#include "router/label.hh"
#include "router/label_heap.hh"

class CentralizedLabel
{
private:
  Vertex vertex;
  std::vector<Label> labels;
  idx numImprovements;
  num minCost;

public:
  CentralizedLabel(const Vertex& vertex,
                   const std::vector<Label>& labels)
    : vertex(vertex),
      labels(labels),
      numImprovements(0),
      minCost(inf)
  {
    for(const Label& label : labels)
    {
      minCost = std::min(minCost, label.getCost());
    }
  }

  CentralizedLabel()
    : numImprovements(0),
      minCost(inf)
  {}

  const Vertex& getVertex() const
  {
    return vertex;
  }

  const std::vector<Label>& getLabels() const
  {
    return labels;
  }

  num getMinCost() const
  {
    return minCost;
  }

  idx getNumImprovements() const
  {
    return numImprovements;
  }

  void resetNumImprovements()
  {
    numImprovements = 0;
  }

  void addImprovements(idx improvements)
  {
    numImprovements += improvements;
  }

  bool operator>(const CentralizedLabel& other)
  {
    assert(labels.size() == other.labels.size());

    if(numImprovements != other.numImprovements)
    {
      return numImprovements < other.numImprovements;
    }

    return minCost > other.minCost;
  }

  template <Direction direction = Direction::OUTGOING>
  idx update(const CentralizedLabel& other,
             const Edge& edge,
             const num cost)
  {
    idx result = 0;

    if(*this)
    {
      assert(labels.size() == other.labels.size());
      assert(edge.getEndpoint(direction) == vertex);
      assert(edge.getEndpoint(opposite(direction)) == other.vertex);

      for(uint i = 0; i < labels.size(); ++i)
      {
        const Label& otherLabel = other.labels[i];

        if(otherLabel.getCost() == inf)
        {
          continue;
        }

        const num nextCost = otherLabel.getCost() + cost;

        Label& label = labels[i];

        if(nextCost < label.getCost())
        {
          ++result;
          label = Label(vertex, edge, nextCost);
        }

        minCost = std::min(minCost, label.getCost());
      }
    }
    else
    {
      assert(edge.getEndpoint(opposite(direction)) == other.vertex);

      vertex = edge.getEndpoint(direction);

      for(const Label& otherLabel : other.labels)
      {
        if(otherLabel.getCost() == inf)
        {
          labels.push_back(Label());
        }
        else
        {
          const num nextCost = otherLabel.getCost() + cost;

          ++result;
          labels.push_back(Label(vertex, edge, nextCost));

          minCost = std::min(minCost, nextCost);
        }
      }
    }

    if(debuggingEnabled())
    {
      assert(*this);
      for(const Label& label : labels)
      {
        assert(label.getCost() >= minCost);
      }
    }

    return result;
  }

  operator bool() const
  {
    return labels.size() > 0;
  }

};

class CentralizedHeap
{
private:

  struct LargeLabel;

  struct Reference
  {
    Reference(LargeLabel* label)
      : label(label)
    {}

    LargeLabel* label;
    bool operator>(const Reference& other) const;
  };

  typedef typename boost::heap::d_ary_heap<Reference,
                                           boost::heap::mutable_<true>,
                                           boost::heap::compare<std::greater<Reference>>,
                                           boost::heap::arity<2>> Heap;

  typedef typename Heap::handle_type Handle;

  Heap heap;
  VertexMap<LargeLabel> labels;

  struct LargeLabel
  {
    CentralizedLabel label;
    Handle handle;
    bool inHeap;

    LargeLabel()
      : inHeap(false)
    {
    }
  };

public:
  CentralizedHeap(const Graph& graph)
    : labels(graph, LargeLabel())
  {}

  CentralizedLabel& extractMin()
  {
    assert(!isEmpty());

    Reference reference = heap.top();
    heap.pop();

    LargeLabel& largeLabel = *(reference.label);
    largeLabel.inHeap = false;

    return largeLabel.label;
  }

  void update(const Vertex& vertex)
  {
    LargeLabel& largeLabel = labels(vertex);

    if(largeLabel.inHeap)
    {
      Handle handle = largeLabel.handle;
      heap.update(handle, Reference(&largeLabel));
    }
    else
    {
      Handle handle = heap.push(Reference(&largeLabel));
      largeLabel.handle = handle;
      largeLabel.inHeap = true;
    }
  }

  bool contains(const Vertex& vertex) const
  {
    return labels(vertex).inHeap;
  }

  const CentralizedLabel& getLabel(const Vertex& vertex) const
  {
    return labels(vertex).label;
  }

  CentralizedLabel& getLabel(const Vertex& vertex)
  {
    return labels(vertex).label;
  }

  bool isEmpty() const
  {
    return heap.empty();
  }

};

bool CentralizedHeap::Reference::operator>(const CentralizedHeap::Reference& other) const
{
  return (label->label) > (other.label->label);
}


CentralizedPreprocessor::CentralizedPreprocessor(const Graph& graph,
                                                 const EdgeFunc<num>& costs,
                                                 const Partition& partition)
  : graph(graph),
    costs(costs),
    partition(partition),
    outgoingFlags(graph, partition),
    incomingFlags(graph, partition)
{
  idx boundarySize = 0;

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
    }
  }

  Log(info) << "Found " << boundarySize
            << " overlapping edges";

  Log(info) << "Setting outgoing flags";

  for(const Region& region : partition.getRegions())
  {
    setFlags<Direction::OUTGOING>(region);
  }

  Log(info) << "Setting incoming flags";

  for(const Region& region : partition.getRegions())
  {
    setFlags<Direction::INCOMING>(region);
  }

  Log(info) << "Outgoing flags: " << outgoingFlags
            << ", incoming flags: " << incomingFlags;
}

template<Direction direction>
void CentralizedPreprocessor::setFlags(const Region& region)
{
  ArcFlags& arcFlags = (direction == Direction::OUTGOING) ?
    outgoingFlags : incomingFlags;

  std::unordered_set<Vertex> vertexSet;

  for(const Vertex& vertex : region.getVertices())
  {
    for(const Edge& edge : graph.getAdjacentEdges(vertex))
    {
      const Vertex& other = edge.getOpposite(vertex);

      if(partition.getRegion(other) != region)
      {
        vertexSet.insert(vertex);
        //arcFlags.setFlag(edge, region);
      }
    }
  }

  for(const Vertex& vertex : region.getVertices())
  {
    for(const Edge& edge : graph.getEdges(vertex, opposite(direction)))
    {
      arcFlags.setFlag(edge, region);
    }
  }

  std::vector<Vertex> vertices(vertexSet.begin(),
                               vertexSet.end());

  auto filter = partition.regionFilter(region);

  CentralizedHeap centralizedHeap(graph);

  for(const Vertex& target : vertices)
  {
    assert(partition.getRegion(target) == region);

    LabelHeap<SimpleLabel> heap(graph);

    heap.update(SimpleLabel(target, 0));

    for(const Vertex& source : vertices)
    {
      assert(partition.getRegion(source) == region);

      while(!heap.isEmpty())
      {
        if(heap.getLabel(source).getState() == State::SETTLED)
        {
          break;
        }

        const SimpleLabel& current = heap.extractMin();

        for(const Edge& edge : graph.getEdges(current.getVertex(),
                                              opposite(direction)))
        {
          if(!filter(edge))
          {
            continue;
          }

          SimpleLabel nextLabel = SimpleLabel(edge.getEndpoint(opposite(direction)),
                                              current.getCost() + costs(edge));

          heap.update(nextLabel);
        }
      }
    }

    std::vector<Label> labels;

    for(const Vertex& source : vertices)
    {
      if(debuggingEnabled())
      {
        Dijkstra dijkstra(graph);
        auto result = (direction == Direction::OUTGOING) ?
          dijkstra.shortestPath(source, target, costs, filter) :
          dijkstra.shortestPath(target, source, costs, filter);

        auto label = heap.getLabel(source);

        if(result.found)
        {
          assert(result.path.satisfies(filter));
          assert(label.getCost() == result.path.cost(costs));
        }
        else
        {
          assert(label.getCost() == inf);
        }
      }

      labels.push_back(Label(target,
                             Edge(),
                             heap.getLabel(source).getCost()));
    }

    assert(labels.size() == vertices.size());

    CentralizedLabel centralizedLabel(target, labels);
    assert(centralizedLabel.getMinCost() == 0);
    centralizedHeap.getLabel(target) = centralizedLabel;
    centralizedHeap.update(target);

  }

  //Log(debug) << "Starting centralized computation";

  while(!centralizedHeap.isEmpty())
  {
    CentralizedLabel& label = centralizedHeap.extractMin();
    const Vertex& vertex = label.getVertex();

    assert(!centralizedHeap.contains(vertex));

    /*
    Log(debug) << "Label has "
               << label.getNumImprovements()
               << " improvements and "
               << label.getMinCost()
               << " min cost";
    */

    label.resetNumImprovements();

    assert(label.getNumImprovements() == 0);

    for(const Edge& edge : graph.getEdges(vertex, direction))
    {
      const Vertex& nextVertex = edge.getEndpoint(direction);

      if(partition.getRegion(nextVertex) == region)
      {
        continue;
      }

      CentralizedLabel& nextLabel = centralizedHeap.getLabel(nextVertex);

      const idx improvements = nextLabel.update<direction>(label, edge, costs(edge));

      if(improvements > 0)
      {
        centralizedHeap.update(nextVertex);
        assert(centralizedHeap.contains(nextVertex));
      }

      label.addImprovements(improvements);
    }
  }

  //Log(debug) << "Finished centralized computation";

  /*
  if(debuggingEnabled())
  {
    for(const Vertex& target : vertices)
    {
      const CentralizedLabel& centralizedLabel = centralizedHeap.getLabel(target);

      for(uint i = 0; i < vertices.size(); ++i)
      {
        const Vertex& source = vertices[i];
        const Label& label = centralizedLabel.getLabels()[i];

        Dijkstra dijkstra(graph);

        auto result = (direction == Direction::OUTGOING) ?
          dijkstra.shortestPath(source, target, costs) :
          dijkstra.shortestPath(target, source, costs);

        if(result.found)
        {
          num actualCost = result.path.cost(costs);
          assert(label.getCost() == actualCost);
        }
        else
        {
          assert(label.getCost() == inf);
        }
      }
    }
  }
  */

  for(const Vertex& vertex : graph.getVertices())
  {
    if(partition.getRegion(vertex) == region)
    {
      continue;
    }

    const CentralizedLabel& centralizedLabel = centralizedHeap.getLabel(vertex);

    /*
    if(debuggingEnabled())
    {
      for(uint i = 0; i < vertices.size(); ++i)
      {
        const Label& label = centralizedLabel.getLabels()[i];

        if(label.getCost() == inf)
        {
          continue;
        }

        Label current = label;
        num totalCost = 0;

        while(std::find(vertexSet.begin(),
                        vertexSet.end(),
                        current.getVertex()) == vertexSet.end())
        {
          const Edge& edge = current.getEdge();
          Vertex previous = edge.getEndpoint(opposite(direction));
          current = centralizedHeap.getLabel(previous).getLabels()[i];
          totalCost += costs(edge);
        }

        assert(totalCost == label.getCost() - current.getCost());
      }
    }
    */

    /*
    for(uint i = 0; i < vertices.size(); ++i)
    {
      const Vertex& boundaryVertex = vertices[i];

      Dijkstra dijkstra(graph);

      auto result = (direction == Direction::OUTGOING) ?
        dijkstra.shortestPath(boundaryVertex, vertex, costs) :
        dijkstra.shortestPath(vertex, boundaryVertex, costs);

      const Label& label = centralizedLabel.getLabels()[i];

      if(result.found)
      {
        assert(label.getCost() == result.path.cost(costs));
      }
      else
      {
        assert(label.getCost() == inf);
      }
    }
    */

    /*
    if(std::find(vertices.begin(), vertices.end(), vertex) != vertices.end())
    {
      continue;
    }
    */

    if(!centralizedLabel)
    {
      continue;
    }

    //assert(centralizedLabel.getNumImprovements() == 0);

    for(const Label& label : centralizedLabel.getLabels())
    {
      if(label.getCost() != inf)
      {
        arcFlags.setFlag(label.getEdge(), region);
      }
    }
  }
}

ArcFlagRouter CentralizedPreprocessor::getRouter() const
{
  return ArcFlagRouter(graph,
                       outgoingFlags,
                       incomingFlags,
                       partition);
}

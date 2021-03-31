#include "graph_reader.hh"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "log.hh"

#include "graph.pb.h"

const int maxSize = std::numeric_limits<int32_t>::max();

ReadResult GraphReader::readGraph(std::istream& in)
{
  using namespace google::protobuf::io;

  Protobuf::Graph PBFGraph;

  Log(info) << "Reading graph";

  IstreamInputStream* input = new IstreamInputStream(&in);
  CodedInputStream* codedInput = new CodedInputStream(input);

  codedInput->SetTotalBytesLimit(maxSize, maxSize);

  if(!in)
  {
    throw std::runtime_error("Could not open input");
  }
  else if(!PBFGraph.MergeFromCodedStream(codedInput))
  {
    throw std::runtime_error("Failed to parse input");
  }

  //std::cout << PBFGraph.DebugString();

  std::unordered_map<long, Vertex> vertexMap;
  std::vector<Vertex> vertices;

  for(int i = 0; i < PBFGraph.vertices_size();++i)
  {
    const Protobuf::Vertex& PBFVertex = PBFGraph.vertices(i);

    Vertex vertex(i);

    vertexMap.insert(std::make_pair(PBFVertex.id(), vertex));
    vertices.push_back(vertex);
  }

  std::vector<Edge> edges;

  for(int i = 0; i < PBFGraph.edges_size();++i)
  {
    const Protobuf::Edge& PBFEdge = PBFGraph.edges(i);
    Vertex source = vertexMap.find(PBFEdge.source())->second;
    Vertex target = vertexMap.find(PBFEdge.target())->second;

    edges.push_back(Edge(source, target, i));
  }

  Graph graph(vertices.size(), edges);

  bool length_found = false, speed_limit_found = false;

  EdgeMap<num> costs(graph, 0);
  EdgeMap<num> deviations(graph, 0);

  EdgeMap<num> length(graph, 0), speed_limit(graph, 0);

  for(int i = 0; i < PBFGraph.tags_size(); ++i)
  {
    const Protobuf::Tag& tag = PBFGraph.tags(i);

    if(!tag.has_float_values())
    {
      continue;
    }

    const Protobuf::FloatTagValues& values = tag.float_values();

    if("length" == tag.name())
    {
      // length: given in meters
      length_found = true;

      for(int i = 0; i < values.values_size(); ++i)
      {
        length(edges[i]) = values.values(i);
      }
    }
    else if("speed_limit" == tag.name())
    {
      // speed limit: given in km/h
      speed_limit_found = true;

      for(int i = 0; i < values.values_size(); ++i)
      {
        speed_limit(edges[i]) = values.values(i);
      }
    }
  }

  for(const Edge& edge : graph.getEdges())
  {
    costs(edge) = (int) std::round(length(edge) / (speed_limit(edge) / 3.6f));
  }

  for(const Edge& edge : graph.getEdges())
  {
    deviations(edge) = (int) (std::round(length(edge) / (std::min(speed_limit(edge), 10) / 3.6f)))
      - costs(edge);
  }

  VertexMap<Point> points(graph, Point(0, 0));

  for(int i = 0; i < PBFGraph.vertices_size();++i)
  {
    const Protobuf::Vertex& PBFVertex = PBFGraph.vertices(i);

    points(vertexMap.find(PBFVertex.id())->second)
      = Point(PBFVertex.lon(), PBFVertex.lat());
  }

  if(!(length_found and speed_limit_found))
  {
    throw std::runtime_error("Could not find required tags");
  }

  Log(info) << "Read a graph with "
            << graph.getVertices().size()
            << " vertices and "
            << graph.getEdges().size()
            << " edges";

  delete codedInput;
  delete input;

  return ReadResult(graph, costs, deviations, points);
}

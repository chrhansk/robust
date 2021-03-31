#include "abstract_contraction_preprocessor.hh"

const num AbstractContractionPreprocessor::INVALID = (num) -1;

std::vector<ContractionResult>
AbstractContractionPreprocessor::contractVertex(
  const Vertex& vertex,
  Graph& overlayGraph,
  EdgeMap<num>& overlayCosts,
  EdgeMap<EdgePair>& originalEdges,
  const VertexFunc<bool>& contracted,
  const std::vector<ContractionPair>& pairs)
{
  std::vector<ContractionResult> results;

  for(const ContractionPair& pair : pairs)
  {
    const Vertex& source = pair.getSource();
    const Vertex& target = pair.getTarget();

    assert(!contracted(source));
    assert(!contracted(target));

    const Path& path = pair.getDefaultPath();

    assert(path.getEdges().size() == 2);
    assert(path.contains(vertex));

    num cost = path.cost(overlayCosts);
    Edge edge = overlayGraph.addEdge(source, target);

    overlayCosts.extend(edge, cost);
    originalEdges.extend(edge,
                         EdgePair(*(path.getEdges().begin()),
                                  *(path.getEdges().rbegin())));

    assert(overlayCosts(edge) == cost);

    results.push_back(ContractionResult(pair, edge));
  }

  return results;
}

#include "arcflag_test.hh"

#include <sstream>

#include "reader/bidirected_arcflag_reader.hh"
#include "writer/bidirected_arcflag_writer.hh"

void testArcFlagEquality(const ArcFlags& first, const ArcFlags& second)
{
  const Partition& partition = first.getPartition();
  const Graph& graph = partition.getGraph();

  for(const Edge& edge : graph.getEdges())
  {
    for(const Region& region : partition.getRegions())
    {
      ASSERT_EQ(first.hasFlag(edge, region), second.hasFlag(edge, region));
    }
  }
}

TEST_F(ArcFlagTest, testWriteFlags)
{
  std::stringstream buf;

  BidirectedArcFlagWriter().writeBidirectedArcFlags(buf,
                                                    preprocessor.getIncomingFlags(),
                                                    preprocessor.getOutgoingFlags());

  auto result = BidirectedArcFlagReader().readBidirectedArcFlags(graph, buf);
  const Partition& resultPartition = *(result.partition);

  ASSERT_EQ(partition.getRegions().size(),
            resultPartition.getRegions().size());

  for(idx i = 0; i < partition.getRegions().size(); ++i)
  {
    ASSERT_EQ(partition.getRegions()[i].getVertices(),
              resultPartition.getRegions()[i].getVertices());
  }

  testArcFlagEquality(preprocessor.getIncomingFlags(), *result.incomingFlags);
  testArcFlagEquality(preprocessor.getOutgoingFlags(), *result.outgoingFlags);
}

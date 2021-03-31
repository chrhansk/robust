#include "bidirected_arcflag_reader.hh"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "log.hh"

#include "graph.pb.h"

#include "partition_parser.hh"
#include "arcflag_parser.hh"

const int maxSize = std::numeric_limits<int32_t>::max();

BidirectedArcFlagResult
BidirectedArcFlagReader::readBidirectedArcFlags(const Graph& graph, std::istream& in)
{
  using namespace google::protobuf::io;

  Protobuf::BidirectionalArcFlags PBFArcFlags;

  Log(info) << "Reading arc flags";

  IstreamInputStream* input = new IstreamInputStream(&in);
  CodedInputStream* codedInput = new CodedInputStream(input);

  codedInput->SetTotalBytesLimit(maxSize, maxSize);

  if(!in)
  {
    throw std::runtime_error("Could not open input");
  }
  else if(!PBFArcFlags.MergeFromCodedStream(codedInput))
  {
    throw std::runtime_error("Failed to parse input");
  }

  auto partition = PartitionParser().parsePartition(graph, PBFArcFlags.partition());
  auto incomingFlags = ArcFlagParser().parseArcFlags(graph, *partition, PBFArcFlags.incoming_flags());
  auto outgoingFlags = ArcFlagParser().parseArcFlags(graph, *partition, PBFArcFlags.outgoing_flags());

  return BidirectedArcFlagResult(std::move(partition),
                                 std::move(incomingFlags),
                                 std::move(outgoingFlags));
}

#include "bidirected_arcflag_writer.hh"

#include "bidirected_arcflag_composer.hh"

void
BidirectedArcFlagWriter::writeBidirectedArcFlags(std::ostream& out,
                                                 const Bidirected<ArcFlags> arcFlags)
{
  writeBidirectedArcFlags(out,
                          arcFlags.get<Direction::INCOMING>(),
                          arcFlags.get<Direction::OUTGOING>());
}

void
BidirectedArcFlagWriter::writeBidirectedArcFlags(std::ostream& out,
                                                 const ArcFlags& incomingFlags,
                                                 const ArcFlags& outgoingFlags)
{
  using namespace google::protobuf::io;

  Protobuf::BidirectionalArcFlags PBFArcFlags;

  BidirectedArcFlagComposer().composeBidirectedArcFlags(incomingFlags,
                                                        outgoingFlags,
                                                        PBFArcFlags);

  PBFArcFlags.SerializeToOstream(&out);
}

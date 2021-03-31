#include "bidirected_arcflag_composer.hh"

#include "log.hh"

#include "arcflag_composer.hh"
#include "partition_composer.hh"

void
BidirectedArcFlagComposer::composeBidirectedArcFlags(const Bidirected<ArcFlags> arcFlags,
                                                     Protobuf::BidirectionalArcFlags& PBFArcFlags)
{
  composeBidirectedArcFlags(arcFlags.get<Direction::INCOMING>(),
                            arcFlags.get<Direction::OUTGOING>(),
                            PBFArcFlags);
}

void
BidirectedArcFlagComposer::composeBidirectedArcFlags(const ArcFlags& incomingFlags,
                                                     const ArcFlags& outgoingFlags,
                                                     Protobuf::BidirectionalArcFlags& PBFArcFlags)
{
  Log(info) << "Composing bidirected arc flags";

  const Partition& partition = outgoingFlags.getPartition();

  assert(partition == incomingFlags.getPartition());

  PartitionComposer().composePartition(partition, *(PBFArcFlags.mutable_partition()));
  ArcFlagComposer().composeArcFlags(incomingFlags, *(PBFArcFlags.mutable_incoming_flags()));
  ArcFlagComposer().composeArcFlags(outgoingFlags, *(PBFArcFlags.mutable_outgoing_flags()));

  Log(info) << "Composed bidirected arc flags";
}

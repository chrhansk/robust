#ifndef VERTEX_PAIR_MAP_HH
#define VERTEX_PAIR_MAP_HH

#include "util.hh"

template <class R>
using VertexPairMap = PairMap<Vertex, R>;

typedef VertexPairMap<std::vector<num>> DistanceMap;


#endif /* VERTEX_PAIR_MAP_HH */

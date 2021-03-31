SET(COMMON_SRC
  log.cc
  util.cc
  arcflags/arcflag_preprocessor.cc
  arcflags/arcflag_router.cc
  arcflags/arcflags.cc
  arcflags/centralized_preprocessor.cc
  arcflags/geometric_partition.cc
  arcflags/metis_partition.cc
  arcflags/partition.cc
  contraction/abstract_contraction_preprocessor.cc
  contraction/contraction_hierarchy.cc
  contraction/contraction_preprocessor.cc
  contraction/edge_count.cc
  contraction/edge_quotient.cc
  contraction/fast_witness_path_search.cc
  contraction/hop_restricted_witness_path_search.cc
  contraction/level_estimation.cc
  contraction/nested_dissection_order.cc
  contraction/parallel_contraction_preprocessor.cc
  contraction/simple_witness_path_search.cc
  graph/edge.cc
  graph/graph.cc
  graph/metis_graph.cc
  graph/vertex.cc
  path/path.cc
  reader/bidirected_arcflag_reader.cc
  reader/graph_reader.cc
  reader/arcflag_parser.cc
  reader/partition_parser.cc
  reader/required_values_reader.cc
  robust/active/active_router.cc
  robust/active/bidirectional_active_router.cc
  robust/active/goal_directed_active_router.cc
  robust/active/simple_active_router.cc
  robust/arcflags/abstract_arcflag_preprocessor.cc
  robust/arcflags/bounded_arcflags.cc
  robust/arcflags/extended_arcflags.cc
  robust/arcflags/fast_arcflag_preprocessor.cc
  robust/arcflags/robust_arcflag_preprocessor.cc
  robust/arcflags/simple_arcflags.cc
  robust/arcflags/value_arcflag_preprocessor.cc
  robust/contraction/abstract_robust_contraction_preprocessor.cc
  robust/contraction/fast_robust_witness_path_search.cc
  robust/contraction/parallel_robust_contraction_preprocessor.cc
  robust/contraction/robust_contraction_hierarchy.cc
  robust/contraction/robust_contraction_pair.cc
  robust/contraction/robust_contraction_preprocessor.cc
  robust/contraction/robust_search_predicate.cc
  robust/contraction/simple_robust_witness_path_search.cc
  robust/contraction/value_count_quotient.cc
  robust/contraction/value_range_quotient.cc
  robust/discard/discarding_preprocessor.cc
  robust/discard/discarding_robust_router.cc
  robust/robust_costs.cc
  robust/robust_router.cc
  robust/robust_utils.cc
  robust/search_interval.cc
  robust/searching_robust_router.cc
  robust/simple_robust_router.cc
  robust/theta/bidirectional_bounding_router.cc
  robust/theta/bidirectional_goal_directed_router.cc
  robust/theta/bounding_router.cc
  robust/theta/goal_directed_bounding_router.cc
  robust/theta/goal_directed_router.cc
  robust/theta/potential.cc
  robust/theta/simple_theta_router.cc
  robust/theta/stateful_theta_router.cc
  robust/theta/theta_router.cc
  robust/theta_tree.cc
  robust/value_range.cc
  robust/values/abstract_value_preprocessor.cc
  robust/values/bucket_value_preprocessor.cc
  robust/values/fast_value_preprocessor.cc
  robust/values/outer_value_preprocessor.cc
  robust/values/refining_value_preprocessor.cc
  robust/values/robust_value_router.cc
  robust/values/simple_value_preprocessor.cc
  robust/values/value_preprocessor.cc
  router/dijkstra_rank.cc
  router/router.cc
  writer/bidirected_arcflag_composer.cc
  writer/bidirected_arcflag_writer.cc
  writer/arcflag_composer.cc
  writer/partition_composer.cc
  writer/required_values_writer.cc)

CONFIGURE_FILE(defs.hh.in ${CMAKE_BINARY_DIR}/defs.hh)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/)

ADD_LIBRARY(common ${COMMON_SRC})

IF(THREADS_HAVE_PTHREAD_ARG)
  SET_PROPERTY(TARGET common PROPERTY COMPILE_OPTIONS "-pthread")
  SET_PROPERTY(TARGET common PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
ENDIF()

TARGET_LINK_LIBRARIES(common proto
  TBB::tbb
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  ${METIS_LIBRARY}
  ${PROTOBUF_LIBRARY})

ADD_EXECUTABLE(main main.cc)

TARGET_LINK_LIBRARIES(main common)

ADD_EXECUTABLE(value_preprocessor value_preprocessor.cc)

TARGET_LINK_LIBRARIES(value_preprocessor common)

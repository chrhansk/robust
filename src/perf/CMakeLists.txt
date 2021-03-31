ADD_LIBRARY(perf_common
  benchmark_config.cc
  robust/values/value_benchmark.cc
  sample_collector.cc
  sample_benchmark.cc)

ADD_CUSTOM_TARGET(collect)

TARGET_LINK_LIBRARIES(perf_common
  common
  proto
  ${PROTOBUF_LIBRARY})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FUNCTION(ADD_COLLECT_BENCHMARK BENCHMARK_TYPE BENCHMARK_NAME)

  GET_FILENAME_COMPONENT(BASE_NAME ${BENCHMARK_NAME} NAME)
  SET(EXECUTABLE_NAME "${BENCHMARK_TYPE}_${BASE_NAME}")

  ADD_EXECUTABLE(${EXECUTABLE_NAME} ${BENCHMARK_NAME}.cc)

  TARGET_LINK_LIBRARIES(${EXECUTABLE_NAME}
    perf_common)

  ADD_CUSTOM_TARGET(collect_${EXECUTABLE_NAME}
    COMMAND ${EXECUTABLE_NAME} > ${CMAKE_BINARY_DIR}/collect_${EXECUTABLE_NAME}.csv
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${EXECUTABLE_NAME})

  ADD_DEPENDENCIES(collect collect_${EXECUTABLE_NAME})
ENDFUNCTION()

ADD_COLLECT_BENCHMARK(time robust/time/theta/bidirectional_bounding_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/theta/bidirectional_goal_directed_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/theta/bounding_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/theta/goal_directed_bounding_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/theta/goal_directed_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/theta/simple_theta_router_benchmark)

ADD_COLLECT_BENCHMARK(time robust/time/bidirectional_active_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/goal_directed_active_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/goal_directed_searching_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/simple_active_router_benchmark)
ADD_COLLECT_BENCHMARK(time robust/time/simple_searching_router_benchmark)

ADD_COLLECT_BENCHMARK(values robust/values/bidirectional_active_router_benchmark)
ADD_COLLECT_BENCHMARK(values robust/values/simple_robust_router_benchmark)
ADD_COLLECT_BENCHMARK(values robust/values/simple_active_router_benchmark)
ADD_COLLECT_BENCHMARK(values robust/values/simple_searching_router_benchmark)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/benchmark.json
  ${CMAKE_BINARY_DIR} COPYONLY)

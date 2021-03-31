ADD_LIBRARY(test_common
  basic_test.cc
  robust/robust_router_test.cc
  robust/values/value_preprocessor_test.cc)

TARGET_LINK_LIBRARIES(test_common
  common
  ${GTEST_LIBRARIES}
  ${GTEST_MAIN_LIBRARIES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

FUNCTION(ADD_UNIT_TEST TEST_NAME)
  GET_FILENAME_COMPONENT(BASE_NAME ${TEST_NAME} NAME)
  ADD_EXECUTABLE(${BASE_NAME} ${TEST_NAME}.cc)

  TARGET_LINK_LIBRARIES(${BASE_NAME}
    test_common)

  ADD_TEST(NAME ${BASE_NAME} COMMAND ${BASE_NAME})
ENDFUNCTION()

ADD_UNIT_TEST(arcflags/arcflag_test)
ADD_UNIT_TEST(arcflags/arcflag_write_test)
ADD_UNIT_TEST(contraction/contraction_test)
ADD_UNIT_TEST(robust/contraction/robust_contraction_test)
ADD_UNIT_TEST(robust/robust_router_test)
ADD_UNIT_TEST(robust/robust_utils_test)
ADD_UNIT_TEST(robust/theta_router_test)
ADD_UNIT_TEST(robust/theta_tree_test)
#ADD_UNIT_TEST(robust/value_preprocessor_test)
#ADD_UNIT_TEST(robust/value_router_test)
ADD_UNIT_TEST(router/distance_tree_test)
ADD_UNIT_TEST(router/router_test)

ADD_UNIT_TEST(robust/bounding_router_test)
ADD_UNIT_TEST(robust/searching_router_test)
ADD_UNIT_TEST(robust/tightening_router_test)
ADD_UNIT_TEST(robust/bidirectional_active_router_test)
ADD_UNIT_TEST(robust/goal_directed_active_router_test)
ADD_UNIT_TEST(robust/simple_active_router_test)

ADD_UNIT_TEST(robust/active/arcflag_active_router_test)

ADD_UNIT_TEST(robust/arcflags/robust_arcflag_preprocessor_test)
ADD_UNIT_TEST(robust/arcflags/value_arcflag_test)

ADD_UNIT_TEST(robust/values/bucket_value_preprocessor_test)
ADD_UNIT_TEST(robust/values/outer_value_preprocessor_test)
ADD_UNIT_TEST(robust/values/refining_value_preprocessor_test)

ADD_UNIT_TEST(robust/discard/discarding_robust_router_test)


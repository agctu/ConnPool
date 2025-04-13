link_libraries(
  GTest::gtest_main
  connpool
)

add_executable(
  connection_pool_test
  connection_pool_test.cpp
  helper.cpp
)

gtest_discover_tests(connection_pool_test)

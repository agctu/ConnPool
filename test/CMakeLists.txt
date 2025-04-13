add_executable(
  connection_pool_test
  connection_pool_test.cpp
)
target_link_libraries(
  connection_pool_test
  GTest::gtest_main
)

gtest_discover_tests(connection_pool_test)

link_libraries(
  GTest::gtest_main
  connpool
)

add_executable(
  connection_pool_test
  connection_pool_test.cpp
)

gtest_discover_tests(connection_pool_test)

add_executable(
  multi_threading_test
  multi_threading_test.cpp
)

gtest_discover_tests(multi_threading_test)

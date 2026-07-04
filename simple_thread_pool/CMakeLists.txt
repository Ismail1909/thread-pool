# simple_thread_pool

add_library(
    simple_thread_pool
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simple_thread_pool.cpp
)

target_include_directories(
    simple_thread_pool
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
  simple_thread_pool_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/simple_thread_pool_tests.cpp
)
target_include_directories(simple_thread_pool_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # to include simple_thread_pool header

if(UNIX AND NOT MSVC)
target_compile_options(simple_thread_pool_tests PRIVATE -fsanitize=thread -g)
target_link_options(simple_thread_pool_tests PRIVATE -fsanitize=thread -g)
endif()

target_link_libraries(
  simple_thread_pool_tests
  GTest::gtest_main
  simple_thread_pool
)

# Add the test binary to gtest.
include(GoogleTest)
gtest_discover_tests(simple_thread_pool_tests)
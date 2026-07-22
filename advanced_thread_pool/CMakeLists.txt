# advanced_thread_pool

add_library(
    advanced_thread_pool
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/advanced_thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/task_queue.cpp
)

target_include_directories(
    advanced_thread_pool
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(
  advanced_thread_pool_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/advanced_thread_pool_tests.cpp
)
target_include_directories(advanced_thread_pool_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # to include advanced_thread_pool header

if(UNIX AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
target_compile_options(advanced_thread_pool_tests PRIVATE -fsanitize=thread -g)
target_link_options(advanced_thread_pool_tests PRIVATE -fsanitize=thread -g)
endif()

target_link_libraries(
  advanced_thread_pool_tests
  GTest::gtest_main
  advanced_thread_pool
)

# Add the test binary to gtest.
include(GoogleTest)
gtest_discover_tests(advanced_thread_pool_tests)
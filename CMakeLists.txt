cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_COLOR_DIAGNOSTICS ON)


project(thread_pool LANGUAGES CXX)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare( # The hash represents a commit, update it regularly
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)

# Enable Google Test
enable_testing()

# Include subprojects
add_subdirectory(simple_thread_pool)


# Add tester main console
add_executable(
    thread_pool_tester
    ${CMAKE_SOURCE_DIR}/main.cpp
)

target_link_libraries(
    thread_pool_tester
    PRIVATE
    simple_thread_pool
)
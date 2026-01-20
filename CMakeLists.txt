cmake_minimum_required(VERSION 3.20)
project(SIMD-Market-Data-Parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mavx512f -mavx512bw")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Parser library
add_library(parser STATIC
    src/parser.cpp
    src/simd_utils.cpp
    src/fix_message.cpp
)

target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Check for AVX-512 support
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512F)
check_cxx_compiler_flag("-mavx512bw" COMPILER_SUPPORTS_AVX512BW)

if(COMPILER_SUPPORTS_AVX512F AND COMPILER_SUPPORTS_AVX512BW)
    target_compile_options(parser PRIVATE -mavx512f -mavx512bw)
    message(STATUS "AVX-512 support enabled")
else()
    message(WARNING "Compiler does not support AVX-512, performance will be degraded")
endif()

# Example executables
add_executable(simple_parse examples/simple_parse.cpp)
target_link_libraries(simple_parse PRIVATE parser)

add_executable(advanced_usage examples/advanced_usage.cpp)
target_link_libraries(advanced_usage PRIVATE parser)

# Benchmarks (optional - requires Google Benchmark)
find_package(benchmark QUIET)
if(benchmark_FOUND)
    add_executable(benchmark_parser benchmarks/benchmark_parser.cpp)
    target_include_directories(benchmark_parser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks
    )
    target_link_libraries(benchmark_parser PRIVATE parser benchmark::benchmark pthread)
    message(STATUS "Google Benchmark found - building benchmarks")
else()
    message(STATUS "Google Benchmark not found - skipping benchmarks (optional)")
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ flags (Release): ${CMAKE_CXX_FLAGS_RELEASE}")

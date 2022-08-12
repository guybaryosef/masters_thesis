cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(BINARY_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

########## start - Download gtest & google benchmark ##########
include(FetchContent)
if (MSVC)
  set(gtest_force_shared_crt on)     # needed to avoid error LNK2038
endif()
set(BENCHMARK_ENABLE_TESTING off)    # to suppress benchmark internal tests
FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/a3460d1aeeaa43fdf137a6adefef10ba0b59fe4b.zip
)
FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master)

FetchContent_MakeAvailable(googletest
                           googlebenchmark)

########## end - Download gtest & google benchmark ##########

project(GBY_SlotMap)
add_library(GBY_SlotMap "")

find_package(Threads REQUIRED)
set_target_properties(GBY_SlotMap PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(GBY_SlotMap PRIVATE -Wall -Wextra -Wno-unknown-pragmas)

include_directories(
    src
)

add_subdirectory(src)
add_subdirectory(SG14)

enable_testing()
add_subdirectory(test)

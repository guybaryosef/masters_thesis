cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)

project(Guy_SlotMap)

find_package(Threads REQUIRED)

set(BINARY_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})


########## start - Download gtest ##########
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/a3460d1aeeaa43fdf137a6adefef10ba0b59fe4b.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
########## end - Download gtest ##########


add_executable(SlotMap)

target_link_libraries(SlotMap 
    sg14
)

include_directories(
    src
    src/lock_free_impl   
)

add_subdirectory(src)
add_subdirectory(SG14)

enable_testing()
add_subdirectory(test)
cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

project(Guy_SlotMap)

find_package(Threads REQUIRED)


set(BINARY_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})


add_executable(SlotMap)
target_link_libraries(SlotMap sg14)

include_directories(
    src
    src/lock_free_impl   
)

add_subdirectory(src)
add_subdirectory(SG14)




add_executable(GBY_SlotMap_MicroBenchmarks_inserts
    benchmarksMain.cpp
    insert.cpp
)

target_link_libraries(GBY_SlotMap_MicroBenchmarks_inserts
    gtest
    benchmark::benchmark
    sg14
    GBY_SlotMap
)

add_executable(GBY_SlotMap_MicroBenchmarks_iterate
    benchmarksMain.cpp
    iterate.cpp
)

target_link_libraries(GBY_SlotMap_MicroBenchmarks_iterate
    gtest
    benchmark::benchmark
    sg14
    GBY_SlotMap
)

add_executable(GBY_SlotMap_MicroBenchmarks_erase
    benchmarksMain.cpp
    erase.cpp
)

target_link_libraries(GBY_SlotMap_MicroBenchmarks_erase
    gtest
    benchmark::benchmark
    sg14
    GBY_SlotMap
)

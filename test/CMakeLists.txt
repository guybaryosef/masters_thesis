


add_executable(GBY_SlotMap_RegressionTests
    main.cpp
    RegressionTestHelpers.h
    RegressionTestHelpers.cpp
)

target_link_libraries(GBY_SlotMap_RegressionTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

add_executable(GBY_SlotMap_UnitTests
    main.cpp
)

target_link_libraries(GBY_SlotMap_UnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(GBY_SlotMap_UnitTests)
gtest_discover_tests(GBY_SlotMap_RegressionTests)


# all benchmarking
add_subdirectory(benchmarks)

# the different slot maps
add_subdirectory(LockedSlotMap)
add_subdirectory(LockFreeDynamicSlotMap)
add_subdirectory(LockFreeConstSizedSlotMap)
add_subdirectory(OptimizedLockedSlotMap)

# internal helper data structures
add_subdirectory(LockFreeVector)
add_subdirectory(InternalVector)

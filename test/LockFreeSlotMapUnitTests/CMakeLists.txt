
add_executable(LockFreeSlotMapUnitTests
    ../main.cpp
    LockFreeMapUnit.cpp
)

target_link_libraries(LockFreeSlotMapUnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockFreeSlotMapUnitTests)

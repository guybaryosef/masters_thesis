
add_executable(LockedSlotMapUnitTests
    ../main.cpp
    LockedMapUnit.cpp
)

target_link_libraries(LockedSlotMapUnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockedSlotMapUnitTests)

add_executable(LockedSlotMapUnitTest
    LockedSlotMapUnitTest.cpp
)

target_link_libraries(LockedSlotMapUnitTest
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockedSlotMapUnitTest)
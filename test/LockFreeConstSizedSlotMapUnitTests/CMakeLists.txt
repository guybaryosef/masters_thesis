
add_executable(LockFreeConstSizedSlotMapUnitTests
    ../main.cpp
    LockFreeConstSizedMapUnit.cpp
)

target_link_libraries(LockFreeConstSizedSlotMapUnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockFreeConstSizedSlotMapUnitTests)

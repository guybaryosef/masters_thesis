
add_executable(SlotMapUnitTests
    ../main.cpp
    LockedMapUnit.cpp
    LockFreeMapUnit.cpp
    LockFreeConstSizedMapUnit.cpp
)

target_link_libraries(SlotMapUnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(SlotMapUnitTests)

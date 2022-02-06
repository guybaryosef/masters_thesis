
add_executable(LockFreeconstSizedSlotMapUnitTests
    ../main.cpp
    LockFreeConstSizedMapUnit.cpp
)

target_link_libraries(LockFreeconstSizedSlotMapUnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockFreeconstSizedSlotMapUnitTests)

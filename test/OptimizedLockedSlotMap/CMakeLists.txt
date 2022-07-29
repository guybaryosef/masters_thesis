
add_executable(OptimizedLocked_UnitTests
    ../main.cpp
    UnitTests.cpp
)

target_link_libraries(OptimizedLocked_UnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(OptimizedLocked_UnitTests)

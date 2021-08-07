
add_executable(SlotMapRegressionTests
    SlotMapRegressionTests.cpp
    LockedMapRegression.cpp
    LockFreeConstSizedMapRegression.cpp
)

target_link_libraries(SlotMapRegressionTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(SlotMapRegressionTests)

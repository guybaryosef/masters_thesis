
add_executable(Locked_UnitTests
    ../main.cpp
    UnitTests.cpp
)

target_link_libraries(Locked_UnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

add_executable(Locked_RegressionTests
    ../main.cpp
    RegressionTests.cpp
)
target_link_libraries(Locked_RegressionTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)


include(GoogleTest)
gtest_discover_tests(Locked_UnitTests)
gtest_discover_tests(Locked_RegressionTests)

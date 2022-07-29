
add_executable(LockFreeConstSized_UnitTests
    ../main.cpp
    UnitTests.cpp
)
target_link_libraries(LockFreeConstSized_UnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

add_executable(LockFreeConstSized_RegressionTests
    ../main.cpp
    RegressionTests.cpp
)
target_link_libraries(LockFreeConstSized_RegressionTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockFreeConstSized_UnitTests)
gtest_discover_tests(LockFreeConstSized_RegressionTests)

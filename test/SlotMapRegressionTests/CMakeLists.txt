
add_executable(LockedMapRegressions
    ../main.cpp
    LockedMapRegression.cpp
)
target_link_libraries(LockedMapRegressions
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

add_executable(LockFreeConstSizedMapRegressions
    ../main.cpp
    LockFreeConstSizedMapRegression.cpp
)
target_link_libraries(LockFreeConstSizedMapRegressions
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockedMapRegressions)
gtest_discover_tests(LockFreeConstSizedMapRegressions)

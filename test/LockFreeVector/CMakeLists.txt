add_executable(LockFreeVector_UnitTests
    ../main.cpp
    UnitTests.cpp
)

target_link_libraries(LockFreeVector_UnitTests
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(LockFreeVector_UnitTests)
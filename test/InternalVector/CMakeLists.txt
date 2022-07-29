add_executable(InternalVector_UnitTests
    ../main.cpp
    UnitTests.cpp
)

target_link_libraries(InternalVector_UnitTests
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(InternalVector_UnitTests)
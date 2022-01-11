add_executable(LockFreeVectorUnitTests
    ../main.cpp
    LockFreeVectorUnitTests.cpp
)

target_link_libraries(LockFreeVectorUnitTests
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(LockFreeVectorUnitTests)
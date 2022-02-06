add_executable(InternalVectorUnitTests
    ../main.cpp
    InternalVectorUnitTests.cpp
)

target_link_libraries(InternalVectorUnitTests
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(InternalVectorUnitTests)
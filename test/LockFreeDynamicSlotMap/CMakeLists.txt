
add_executable(LockFreeDynamic_UnitTests
    ../main.cpp
    UnitTests.cpp
)

target_link_libraries(LockFreeDynamic_UnitTests
    sg14
    gtest
    gtest_main
    GBY_SlotMap
)

include(GoogleTest)
gtest_discover_tests(LockFreeDynamic_UnitTests)

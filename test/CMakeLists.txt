include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(recovery_scheduler_tests
    scheduler_test.cpp
)

target_link_libraries(recovery_scheduler_tests
    PRIVATE
        recovery_scheduler
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(recovery_scheduler_tests)

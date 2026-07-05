cmake_minimum_required(VERSION 3.15)
project(serviceRecovery_handler VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    include
    include/recovery
)

add_executable(lib_test_scheduler
    src/main.cpp
    src/scheduler.cpp
    src/servicediscoverystate.cpp
)

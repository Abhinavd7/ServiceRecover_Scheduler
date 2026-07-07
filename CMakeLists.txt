cmake_minimum_required(VERSION 3.15)
project(service_recovery_scheduler VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared library (.so on Linux). BUILD_RPATH lets build-tree binaries find it;
# INSTALL_RPATH to support deployment next to the .so ($ORIGIN).
set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}")
set(CMAKE_INSTALL_RPATH "$ORIGIN")

add_library(recovery_scheduler SHARED
    src/scheduler.cpp
    src/servicerecoverystate.cpp
    src/irecoveryaction.cpp
)

target_include_directories(recovery_scheduler
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/recovery
)

add_executable(recovery_demo src/main.cpp)
target_link_libraries(recovery_demo PRIVATE recovery_scheduler)

enable_testing()
add_subdirectory(test)

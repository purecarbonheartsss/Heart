cmake_minimum_required(VERSION 3.20)
project(Heart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for all targets
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Injector executable
add_executable(Heart src/main.cpp)

# DLL target
add_library(dll SHARED
    dll/dllmain.cpp
    dll/hooks.cpp
    dll/pipe_server.cpp
)
target_link_libraries(dll PRIVATE mincore)

cmake_minimum_required(VERSION 3.20)
project(HeartInjector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- INJECTOR EXECUTABLE ---
add_executable(injector
    src/main.cpp
)

target_include_directories(injector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- DLL LIBRARY ---
add_library(dll SHARED
    dll/dllmain.cpp
    dll/hooks.cpp
    dll/pipe_server.cpp
)

target_include_directories(dll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dll)

# Ensure DLL is built as x64
set_target_properties(dll PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Set output names
set_target_properties(injector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

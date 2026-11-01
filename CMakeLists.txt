cmake_minimum_required(VERSION 3.15)

project(CarbonHeart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# Windows / SDK definitions shared by both targets.
set(CARBONHEART_WINDOWS_DEFINITIONS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
)

# Main Dear ImGui + Direct3D 11 GUI.
add_executable(CarbonHeart WIN32
    src/main.cpp
    src/process_info.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(CarbonHeart PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(CarbonHeart PRIVATE ${CARBONHEART_WINDOWS_DEFINITIONS})
target_link_libraries(CarbonHeart PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    user32
    gdi32
    imm32
)

# Console diagnostic: checks for the Roblox process and prints its PID.
add_executable(HeartProcessCheck
    src/injector.cpp
    src/process_info.cpp
)

target_include_directories(HeartProcessCheck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_definitions(HeartProcessCheck PRIVATE ${CARBONHEART_WINDOWS_DEFINITIONS})

cmake_minimum_required(VERSION 3.15)
project(CarbonHeart)

set(CMAKE_CXX_STANDARD 17)

# Fetch ImGui automatically
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# Add our executable
add_executable(CarbonHeart 
    src/main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

# Include directories for ImGui
target_include_directories(CarbonHeart PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Link Windows libraries (DirectX 11, DWM for transparency, d3dcompiler for ImGui)
target_link_libraries(CarbonHeart PRIVATE d3d11 dxgi dwmapi d3dcompiler)

# Hide the console window for a clean UI look
set_target_properties(CarbonHeart PROPERTIES WIN32_EXECUTABLE TRUE)

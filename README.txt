CARBON HEART - COMPLETE SOURCE REPLACEMENT

Project files:
  CMakeLists.txt
  src/main.cpp
  .github/workflows/build.yml

To use:
1. Replace your repository's CMakeLists.txt with this project's CMakeLists.txt.
2. Replace src/main.cpp with this project's src/main.cpp.
3. Replace .github/workflows/build.yml with this project's workflow.
4. Commit and push to the main branch, or run the workflow manually from GitHub Actions.

The GitHub Actions build downloads Dear ImGui v1.90.1, compiles the Win32 and Direct3D 11
backends, and uploads build/Release/CarbonHeart.exe as the CarbonHeart-EXE artifact.

Note: This is a complete source/build setup, but it has not been compiled on a Windows
runner in this environment. If the GitHub run still fails, its Configure CMake or Build
step will contain the specific compiler/linker diagnostic needed to identify the remaining issue.

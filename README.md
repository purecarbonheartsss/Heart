# Carbon Heart build fix

This project builds two Windows executables:

- `CarbonHeart.exe`: the Dear ImGui / Direct3D 11 interface.
- `HeartProcessCheck.exe`: a console diagnostic that checks whether `RobloxPlayerBeta.exe` is running and prints its PID.

The shared `src/process_info.cpp` implementation is compiled into both targets. This fixes the previous unresolved-symbol problem: declarations in one executable cannot be satisfied by source code compiled only into a different executable.

The process check only enumerates process names and IDs. It does not open another process, inject code, or attempt to bypass anti-cheat protections.

## Build with GitHub Actions

Push the files to the `main` branch, or run the workflow manually from the Actions tab. Download the `CarbonHeart-UI` and `HeartProcessCheck` artifacts from the successful run.

## Expected project layout

```text
.
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── injector.cpp
│   ├── process_info.cpp
│   └── process_info.h
└── .github/
    └── workflows/
        └── build.yml
```

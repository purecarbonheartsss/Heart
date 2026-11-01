#pragma once

#include <windows.h>

// Returns the process ID of RobloxPlayerBeta.exe, or 0 if it is not running.
// This is process discovery only; it does not open a process handle.
DWORD GetRobloxPID() noexcept;

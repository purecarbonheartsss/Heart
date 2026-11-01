#include "process_info.h"

#include <iostream>

// Standalone diagnostic: reports whether the Roblox process is running.
// It does not open, inject into, or otherwise modify another process.
int main()
{
    const DWORD pid = GetRobloxPID();
    if (pid == 0)
    {
        std::cout << "RobloxPlayerBeta.exe was not found.\n";
        return 1;
    }

    std::cout << "RobloxPlayerBeta.exe is running. PID: " << pid << '\n';
    return 0;
}

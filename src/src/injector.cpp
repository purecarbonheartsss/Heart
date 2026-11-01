#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

// Function to find the Roblox PID using a process snapshot.
DWORD GetRobloxPID() {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);

        if (Process32First(snapshot, &processEntry)) {
            do {
                if (std::wstring(processEntry.szExeFile) == L"RobloxPlayerBeta.exe") {
                    pid = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

// Safely opens a handle to the Roblox process.
// The key here is using PROCESS_ALL_ACCESS - 0x1 to remove PROCESS_TERMINATE.
// This makes the handle look less suspicious to Hyperion's handle introspection.
HANDLE OpenRobloxProcessSafely(DWORD pid) {
    // PROCESS_ALL_ACCESS - 0x1 removes PROCESS_TERMINATE (0x0001)
    // This is a deliberate technique to avoid the anti-cheat flagging the handle.
    return OpenProcess(PROCESS_ALL_ACCESS - 0x1, FALSE, pid);
}

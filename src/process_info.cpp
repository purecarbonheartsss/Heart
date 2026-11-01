#include "process_info.h"

#include <tlhelp32.h>

DWORD GetRobloxPID() noexcept
{
    const HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    DWORD result = 0;
    if (::Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (_wcsicmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0)
            {
                result = entry.th32ProcessID;
                break;
            }
        } while (::Process32NextW(snapshot, &entry));
    }

    ::CloseHandle(snapshot);
    return result;
}

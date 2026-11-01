#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "offsets.h"

// Function Prototypes
DWORD GetRobloxPID();
bool RemapHyperionSection(HANDLE hProcess);
bool ManualMapDLL(HANDLE hProcess, const std::string& dllPath);

int main() {
    std::cout << "[*] Starting Heart Injector...\n";

    // 1. Find Roblox
    DWORD pid = GetRobloxPID();
    if (pid == 0) {
        std::cout << "[!] Roblox not found.\n";
        system("pause");
        return 1;
    }
    std::cout << "[+] Found Roblox PID: " << pid << "\n";

    // 2. Open Process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, pid);
    if (!hProcess) {
        std::cout << "[!] Failed to open process. Error: " << GetLastError() << "\n";
        system("pause");
        return 1;
    }

    // 3. Bypass Hyperion
    std::cout << "[*] Remapping Hyperion Section...\n";
    if (!RemapHyperionSection(hProcess)) {
        std::cout << "[!] Hyperion bypass failed.\n";
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }
    std::cout << "[+] Hyperion Bypassed.\n";

    // 4. Inject DLL
    std::cout << "[*] Injecting DLL...\n";
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string dllPath = path;
    size_t lastSlash = dllPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        dllPath = dllPath.substr(0, lastSlash + 1) + "dll.dll";
    } else {
        dllPath = "dll.dll";
    }

    if (!ManualMapDLL(hProcess, dllPath)) {
        std::cout << "[!] Injection failed.\n";
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }
    std::cout << "[+] DLL Injected Successfully.\n";

    // 5. Cleanup
    CloseHandle(hProcess);
    std::cout << "[*] Done. Press any key to exit.\n";
    system("pause");
    return 0;
}

DWORD GetRobloxPID() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"RobloxPlayerBeta.exe") == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

bool RemapHyperionSection(HANDLE hProcess) {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    auto NtUnmapViewOfSection = (NTSTATUS(WINAPI*)(HANDLE, PVOID))(GetProcAddress(hNtdll, "NtUnmapViewOfSection"));
    auto NtAllocateVirtualMemory = (NTSTATUS(WINAPI*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG))(GetProcAddress(hNtdll, "NtAllocateVirtualMemory"));

    if (!NtUnmapViewOfSection || !NtAllocateVirtualMemory) return false;

    PVOID byfronBase = nullptr; 
    SIZE_T byfronSize = 0;

    if (NtUnmapViewOfSection(hProcess, byfronBase) != 0) return false;

    PVOID newBase = byfronBase;
    SIZE_T regionSize = byfronSize;

    if (NtAllocateVirtualMemory(hProcess, &newBase, 0, &regionSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE) != 0) {
        return false;
    }

    return true;
}

bool ManualMapDLL(HANDLE hProcess, const std::string& dllPath) {
    HANDLE hFile = CreateFileA(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD fileSize = GetFileSize(hFile, NULL);
    std::vector<BYTE> fileBuffer(fileSize);
    DWORD bytesRead;
    ReadFile(hFile, fileBuffer.data(), fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    if (bytesRead != fileSize) return false;

    auto pDosHeader = (PIMAGE_DOS_HEADER)fileBuffer.data();
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

    auto pNtHeaders = (PIMAGE_NT_HEADERS)(fileBuffer.data() + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) return false;

    PVOID remoteBase = VirtualAllocEx(hProcess, nullptr, pNtHeaders->OptionalHeader.SizeOfImage,
                                      MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteBase) return false;

    if (!WriteProcessMemory(hProcess, remoteBase, fileBuffer.data(), pNtHeaders->OptionalHeader.SizeOfHeaders, NULL)) {
        VirtualFreeEx(hProcess, remoteBase, 0, MEM_RELEASE);
        return false;
    }

    auto pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; ++i) {
        auto& section = pSectionHeader[i];
        if (section.SizeOfRawData == 0) continue;

        PVOID sectionDest = (PBYTE)remoteBase + section.VirtualAddress;
        PVOID sectionSrc = fileBuffer.data() + section.PointerToRawData;

        if (!WriteProcessMemory(hProcess, sectionDest, sectionSrc, section.SizeOfRawData, NULL)) {
            VirtualFreeEx(hProcess, remoteBase, 0, MEM_RELEASE);
            return false;
        }
    }

    auto delta = (UINT_PTR)remoteBase - pNtHeaders->OptionalHeader.ImageBase;
    if (delta != 0) {
        auto pRelocDir = &pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        if (pRelocDir->Size > 0) {
            auto pReloc = (PIMAGE_BASE_RELOCATION)((PBYTE)remoteBase + pRelocDir->VirtualAddress);
            while (pReloc->VirtualAddress != 0) {
                auto count = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                auto pRelocData = (PWORD)(pReloc + 1);

                for (DWORD j = 0; j < count; ++j) {
                    auto type = pRelocData[j] >> 12;
                    auto offset = pRelocData[j] & 0xFFF;

                    if (type == IMAGE_REL_BASED_DIR64) {
                        auto patchAddr = (PBYTE)remoteBase + pReloc->VirtualAddress + offset;
                        UINT_PTR original;
                        ReadProcessMemory(hProcess, patchAddr, &original, sizeof(UINT_PTR), NULL);
                        original += delta;
                        WriteProcessMemory(hProcess, patchAddr, &original, sizeof(UINT_PTR), NULL);
                    }
                }
                pReloc = (PIMAGE_BASE_RELOCATION)((PBYTE)pReloc + pReloc->SizeOfBlock);
            }
        }
    }

    auto pEntryPoint = (LPTHREAD_START_ROUTINE)((PBYTE)remoteBase + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, pEntryPoint, remoteBase, 0, nullptr);

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    return true;
}

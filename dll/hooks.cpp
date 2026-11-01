#include "hooks.h"
#include "offsets.h"
#include <windows.h>
#include <vector>
#include <mutex>
#include <string>

static void* originalLuaPCallK = nullptr;
static std::vector<std::string> g_scriptQueue;
static std::mutex g_queueMutex;

void ElevateThreadIdentity(void* L) {
    if (!L) return;
    
    uintptr_t luaStateAddr = reinterpret_cast<uintptr_t>(L);
    uint64_t currentIdentity = 0;
    ReadProcessMemory(GetCurrentProcess(), 
                      reinterpret_cast<void*>(luaStateAddr + Offsets::ThreadIdentityOffset), 
                      &currentIdentity, sizeof(uint64_t), NULL);
                      
    uint64_t newIdentity = Offsets::TrustedIdentity;
    WriteProcessMemory(GetCurrentProcess(), 
                       reinterpret_cast<void*>(luaStateAddr + Offsets::ThreadIdentityOffset), 
                       &newIdentity, sizeof(uint64_t), NULL);
}

int __fastcall HookedLuaPCallK(void* L, int nargs, int nresults, int errfunc, int status) {
    ElevateThreadIdentity(L);
    
    std::lock_guard<std::mutex> lock(g_queueMutex);
    if (!g_scriptQueue.empty()) {
        g_scriptQueue.erase(g_scriptQueue.begin());
    }
    
    return ((int(*)(void*, int, int, int, int))originalLuaPCallK)(L, nargs, nresults, errfunc, status);
}

namespace Hooks {
    bool Initialize() {
        HMODULE hMod = GetModuleHandleW(L"RobloxPlayerBeta.exe");
        if (!hMod) return false;
        
        uintptr_t base = reinterpret_cast<uintptr_t>(hMod);
        void* targetAddress = reinterpret_cast<void*>(base + Offsets::LuaPCallK);
        
        DWORD oldProtect;
        VirtualProtect(targetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        
        memcpy(&originalLuaPCallK, targetAddress, 5);
        
        uintptr_t hookFunc = reinterpret_cast<uintptr_t>(HookedLuaPCallK);
        uintptr_t relativeJump = hookFunc - (reinterpret_cast<uintptr_t>(targetAddress) + 5);
        
        BYTE jmpCode[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
        *(uintptr_t*)&jmpCode[1] = relativeJump;
        
        memcpy(targetAddress, jmpCode, 5);
        VirtualProtect(targetAddress, 5, oldProtect, &oldProtect);
        
        return true;
    }
    
    void QueueScript(const std::string& script) {
        std::lock_guard<std::mutex> lock(g_queueMutex);
        g_scriptQueue.push_back(script);
    }
}

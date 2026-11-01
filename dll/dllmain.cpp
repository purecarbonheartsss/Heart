#include <windows.h>
#include "hooks.h"
#include "pipe_server.h"

DWORD WINAPI MainThread(LPVOID lpParam) {
    Hooks::Initialize();
    PipeServer::Start();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;
    }
    return TRUE;
}

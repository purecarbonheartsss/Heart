#include "pipe_server.h"
#include "hooks.h"
#include <windows.h>
#include <string>

#define PIPE_NAME L"\\\\.\\pipe\\HeartPipe"

namespace PipeServer {
    void Start() {
        SECURITY_ATTRIBUTES sa = {};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = FALSE;
        
        HANDLE hPipe = CreateNamedPipeW(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4096,
            4096,
            0,
            &sa
        );
        
        if (hPipe == INVALID_HANDLE_VALUE) return;
        
        while (true) {
            if (ConnectNamedPipe(hPipe, NULL) != FALSE) {
                DWORD scriptLen = 0;
                DWORD bytesRead = 0;
                
                if (ReadFile(hPipe, &scriptLen, sizeof(scriptLen), &bytesRead, NULL) && bytesRead == sizeof(scriptLen)) {
                    std::string script(scriptLen, '\0');
                    if (ReadFile(hPipe, script.data(), scriptLen, &bytesRead, NULL) && bytesRead == scriptLen) {
                        Hooks::QueueScript(script);
                    }
                }
                DisconnectNamedPipe(hPipe);
            }
        }
        CloseHandle(hPipe);
    }
}

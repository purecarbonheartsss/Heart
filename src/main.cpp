#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// --- Forward declarations for injector functions (we'll implement these in injector.cpp) ---
DWORD GetRobloxPID();
HANDLE OpenRobloxProcessSafely(DWORD pid);

// Direct3D 11 objects.
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// UI state.
static bool g_topMost = false;
static std::vector<std::string> g_consoleLog;

// Forward declarations.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CreateRenderTarget();
void CleanupRenderTarget();
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ApplyCarbonHeartTheme();

void ApplyCarbonHeartTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);

    ImVec4* colors = style.Colors;

    // Silver / black chrome palette.
    colors[ImGuiCol_Text]           = ImVec4(0.85f, 0.85f, 0.90f, 1.00f);
    colors[ImGuiCol_WindowBg]       = ImVec4(0.04f, 0.04f, 0.05f, 0.98f);
    colors[ImGuiCol_Border]         = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
    colors[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg]        = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.60f, 0.60f, 0.65f, 1.00f);
    colors[ImGuiCol_Header]         = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.02f, 0.02f, 0.03f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
}

// Helper to add a message to the on-screen console
void LogToConsole(const std::string& message) {
    g_consoleLog.push_back(message);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"CarbonHeartClass";

    if (!::RegisterClassExW(&wc))
    {
        ::MessageBoxW(nullptr, L"Could not register the window class.", L"Carbon Heart", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"Carbon Heart", WS_POPUP, 100, 100, 800, 500, nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
    {
        ::UnregisterClassW(wc.lpszClassName, hInstance);
        ::MessageBoxW(nullptr, L"Could not create the application window.", L"Carbon Heart", MB_ICONERROR | MB_OK);
        return 1;
    }

    const DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
    (void)::DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, hInstance);
        ::MessageBoxW(nullptr, L"Could not initialise Direct3D 11.", L"Carbon Heart", MB_ICONERROR | MB_OK);
        return 1;
    }

    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ApplyCarbonHeartTheme();

    const bool win32InitOk = ImGui_ImplWin32_Init(hwnd);
    const bool dx11InitOk = win32InitOk && ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    if (!win32InitOk || !dx11InitOk)
    {
        if (win32InitOk) ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, hInstance);
        ::MessageBoxW(nullptr, L"Could not initialise the ImGui backends.", L"Carbon Heart", MB_ICONERROR | MB_OK);
        return 1;
    }

    LogToConsole("> Carbon Heart ready.");

    bool done = false;
    while (!done)
    {
        MSG msg{};
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

        const ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("CarbonHeart_Main", nullptr, windowFlags);

        ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.75f, 1.0f), "CARBON HEART");
        ImGui::SameLine(ImGui::GetWindowWidth() - 40.0f);
        if (ImGui::Button("X", ImVec2(25.0f, 25.0f))) done = true;
        ImGui::Separator();

        if (ImGui::BeginTabBar("MainTabs"))
        {
            if (ImGui::BeginTabItem("Executor"))
            {
                ImGui::TextUnformatted("Status");
                ImGui::SameLine();
                if (ImGui::Button("Connect to Roblox", ImVec2(150, 30))) {
                    DWORD pid = GetRobloxPID();
                    if (pid != 0) {
                        LogToConsole("Found Roblox PID: " + std::to_string(pid));
                        HANDLE hProc = OpenRobloxProcessSafely(pid);
                        if (hProc != NULL) {
                            LogToConsole("✅ SUCCESS: Safely attached to Roblox process.");
                            CloseHandle(hProc); // Close handle for now until injection is ready
                        } else {
                            LogToConsole("❌ ERROR: Could not open process handle.");
                        }
                    } else {
                        LogToConsole("❌ ERROR: Roblox is not running.");
                    }
                }

                ImGui::Separator();
                ImGui::TextUnformatted("Console Output:");
                
                // Draw the console log
                ImGui::BeginChild("Console", ImVec2(0.0f, 300.0f), true);
                for (const auto& log : g_consoleLog) {
                    ImGui::TextUnformatted(log.c_str());
                }
                // Auto-scroll to bottom
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                    ImGui::SetScrollHereY(1.0f);
                }
                ImGui::EndChild();

                if (ImGui::Button("Clear Log", ImVec2(100.0f, 30.0f))) {
                    g_consoleLog.clear();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::TextUnformatted("UI Theme: Carbon Heart (Silver/Black Chrome)");
                if (ImGui::Checkbox("Top Most", &g_topMost)) {
                    ::SetWindowPos(hwnd, g_topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, hInstance);

    return 0;
}

// ... (WndProc, CreateDeviceD3D, and other boilerplate functions remain the same as your original) ...
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() != nullptr && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return 1;
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && g_pSwapChain != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            const UINT width = static_cast<UINT>(LOWORD(lParam));
            const UINT height = static_cast<UINT>(HIWORD(lParam));
            if (width > 0 && height > 0)
            {
                const HRESULT resizeResult = g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
                if (SUCCEEDED(resizeResult)) (void)CreateRenderTarget();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL featureLevel{};

    const HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (FAILED(result)) return false;
    return CreateRenderTarget();
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain != nullptr) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext != nullptr) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice != nullptr) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

bool CreateRenderTarget()
{
    if (g_pSwapChain == nullptr || g_pd3dDevice == nullptr) return false;
    ID3D11Texture2D* backBuffer = nullptr;
    const HRESULT getBufferResult = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(getBufferResult) || backBuffer == nullptr) return false;
    const HRESULT createViewResult = g_pd3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_mainRenderTargetView);
    backBuffer->Release();
    return SUCCEEDED(createViewResult);
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView != nullptr) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

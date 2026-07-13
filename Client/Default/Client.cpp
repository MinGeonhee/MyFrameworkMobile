#include "CMainApp.h"

namespace
{
constexpr wchar_t WINDOW_CLASS_NAME[] = L"MyFrameworkMobileWindow";
constexpr wchar_t WINDOW_TITLE[] = L"MyFramework Mobile";
constexpr uint32 WINDOW_WIDTH = 1280;
constexpr uint32 WINDOW_HEIGHT = 720;

// Client에서는 이 값만 바꾸면 그래픽 API를 선택할 수 있습니다.
constexpr GRAPHIC_API GRAPHIC_BACKEND = GRAPHIC_API::VULKAN;

CMainApp* g_pMainApp = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    switch (iMessage)
    {
    case WM_SIZE:
        if (g_pMainApp != nullptr && wParam != SIZE_MINIMIZED)
        {
            g_pMainApp->Resize(
                static_cast<uint32>(LOWORD(lParam)),
                static_cast<uint32>(HIWORD(lParam)));
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, iMessage, wParam, lParam);
}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int iCmdShow)
{
    WNDCLASSEXW WindowClass{};
    WindowClass.cbSize = sizeof(WNDCLASSEXW);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WndProc;
    WindowClass.hInstance = hInstance;
    WindowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    WindowClass.lpszClassName = WINDOW_CLASS_NAME;

    if (RegisterClassExW(&WindowClass) == 0)
        return -1;

    RECT WindowRect{0, 0, static_cast<LONG>(WINDOW_WIDTH), static_cast<LONG>(WINDOW_HEIGHT)};
    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (hWnd == nullptr)
        return -1;

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    g_pMainApp = CMainApp::Create(hWnd, WINDOW_WIDTH, WINDOW_HEIGHT, GRAPHIC_BACKEND);
    if (g_pMainApp == nullptr)
    {
        MessageBoxW(hWnd, L"그래픽 장치 초기화에 실패했습니다.", L"MyFrameworkMobile", MB_OK | MB_ICONERROR);
        DestroyWindow(hWnd);
        return -1;
    }

    MSG Message{};
    while (Message.message != WM_QUIT)
    {
        if (PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageW(&Message);
        }
        else
        {
            g_pMainApp->Update();
            g_pMainApp->Render();
        }
    }

    delete g_pMainApp;
    g_pMainApp = nullptr;

    UnregisterClassW(WINDOW_CLASS_NAME, hInstance);
    return static_cast<int>(Message.wParam);
}

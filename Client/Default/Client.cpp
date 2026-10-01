#include "CMainApp.h"
#include "CGameManager.h"

#ifdef _DEBUG
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

namespace
{
	constexpr wchar_t WINDOW_CLASS_NAME[] = L"MyFrameworkMobileWindow";
	constexpr wchar_t WINDOW_TITLE[] = L"MyFramework Mobile";

	// Client에서는 이 값만 바꾸면 그래픽 API를 선택할 수 있습니다. (D3D11 / VULKAN)
	// 실행 인자 -d3d11 / -vulkan 으로도 바꿀 수 있습니다.
	GRAPHIC_API GRAPHIC_BACKEND = GRAPHIC_API::VULKAN;

	CMainApp* g_pMainApp = nullptr;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
#ifdef _DEBUG
		if (ImGui_ImplWin32_WndProcHandler(hWnd, iMessage, wParam, lParam))
			return true;
#endif

		switch (iMessage)
		{
		case WM_SIZE:
			if (g_pMainApp != nullptr && wParam != SIZE_MINIMIZED)
				g_pMainApp->Resize(static_cast<uint32>(LOWORD(lParam)), static_cast<uint32>(HIWORD(lParam)));
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

	/* 원본과 같은 경로 규칙("../../Resource/...")을 쓰기 위해 실행 기준 폴더를 Client/Bin 으로 맞춘다 */
	void Set_WorkingDirectory()
	{
		wchar_t szPath[MAX_PATH] = {};
		GetModuleFileNameW(nullptr, szPath, MAX_PATH);
		std::filesystem::path ExePath(szPath);
		std::filesystem::path BinPath = ExePath.parent_path().parent_path();	// Client/Bin/Debug → Client/Bin
		if (std::filesystem::exists(BinPath / L"../../Resource"))
			SetCurrentDirectoryW(BinPath.c_str());
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int iCmdShow)
{
	Set_WorkingDirectory();

	if (lpCmdLine != nullptr)
	{
		const std::wstring wstrCmd = lpCmdLine;
		if (wstrCmd.find(L"-d3d11") != std::wstring::npos)  GRAPHIC_BACKEND = GRAPHIC_API::D3D11;
		if (wstrCmd.find(L"-vulkan") != std::wstring::npos) GRAPHIC_BACKEND = GRAPHIC_API::VULKAN;
	}

	WNDCLASSEXW WindowClass{};
	WindowClass.cbSize = sizeof(WNDCLASSEXW);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = WndProc;
	WindowClass.hInstance = hInstance;
	WindowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	WindowClass.lpszClassName = WINDOW_CLASS_NAME;

	if (RegisterClassExW(&WindowClass) == 0)
		return -1;

	RECT WindowRect{ 0, 0, static_cast<LONG>(Game::WINCX), static_cast<LONG>(Game::WINCY) };
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindowW(WINDOW_CLASS_NAME, WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
		return -1;

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	g_pMainApp = CMainApp::Create(hWnd, hInstance, Game::WINCX, Game::WINCY, GRAPHIC_BACKEND);
	if (g_pMainApp == nullptr)
	{
		MessageBoxW(hWnd, L"엔진 초기화에 실패했습니다.", L"MyFrameworkMobile", MB_OK | MB_ICONERROR);
		DestroyWindow(hWnd);
		return -1;
	}

	CGameManager* pGameManager = CGameManager::GetInstance();
	float fTimeAcc = 0.f;

	MSG Message{};
	while (Message.message != WM_QUIT)
	{
		if (PeekMessageW(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessageW(&Message);
			continue;
		}

		/* 원본 Client 메인 루프 : Timer_Default 누적 후 1/60 초마다 Update / Render */
		pGameManager->TimerManager_Compute_TimeDelta(TEXT("Timer_Default"));
		fTimeAcc += pGameManager->TimerManager_Get_TimeDelta(TEXT("Timer_Default"));

		if (fTimeAcc >= 1.f / 60.0f)
		{
			pGameManager->TimerManager_Compute_TimeDelta(TEXT("Timer_60"));

			g_pMainApp->Update((std::min)(pGameManager->TimerManager_Get_TimeDelta(TEXT("Timer_60")), 0.1f));
			g_pMainApp->Render();

			const uint32 iFPS = pGameManager->TimerManger_Get_FPS(TEXT("Timer_60"));
			wstring wstrTitle = wstring(WINDOW_TITLE) + L"  [" + (GRAPHIC_BACKEND == GRAPHIC_API::D3D11 ? L"D3D11" : L"Vulkan") + L"]  FPS : " + std::to_wstring(iFPS);
			SetWindowTextW(hWnd, wstrTitle.c_str());

			fTimeAcc = 0.f;
		}
	}

	Safe_Release(g_pMainApp);

	UnregisterClassW(WINDOW_CLASS_NAME, hInstance);
	return static_cast<int>(Message.wParam);
}

#include "CMainApp.h"

#include "CGameManager.h"
#include "Game_Entry.h"

#pragma region ImGui Window
#ifdef _DEBUG
#include "CImGuiManager.h"
#include "CGameObjectInfoWindow.h"
#include "CCameraObjectInfoWindow.h"
#include "CLightInfoWindow.h"
#endif // _DEBUG
#pragma endregion

namespace
{
#ifdef _DEBUG
	CImGuiManager* g_pImGuiManager = nullptr;

	/* Game::Render 의 Draw 와 EndFrame 사이에서 ImGui 를 그린다 (원본 CMainApp::Render 순서) */
	void Render_ImGui()
	{
		if (g_pImGuiManager != nullptr)
			g_pImGuiManager->Render();
	}
#endif
}

CMainApp::CMainApp()
	: m_pGameManager(nullptr)
#ifdef _DEBUG
	, m_pImGuiManager(nullptr)
	, m_isOnImGui(true)
#endif
{
}

HRESULT CMainApp::Initialize(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI)
{
	m_pGameManager = CGameManager::GetInstance();
	Safe_AddRef(m_pGameManager);

	if (FAILED(Game::Initialize(hWnd, hInst, iWinSizeX, iWinSizeY, eGraphicAPI, &m_pDevice, &m_pContext)))
		return E_FAIL;

#ifdef _DEBUG
	if (m_isOnImGui)
	{
		m_pImGuiManager = CImGuiManager::GetInstance();
		m_pImGuiManager->Initialize_ImGui(hWnd, m_pDevice, m_pContext, ImGui_Style::LIGHT);
		g_pImGuiManager = m_pImGuiManager;

		/* 엔진에 들어있는 ImGui 디버그 창 */
		m_pImGuiManager->Add_ImGuiWindow(0, CGameObjectInfoWindow::Create(hWnd, m_pDevice, m_pContext));
		m_pImGuiManager->Add_ImGuiWindow(1, CCameraObjectInfoWindow::Create(hWnd, m_pDevice, m_pContext));
		m_pImGuiManager->Add_ImGuiWindow(2, CLightInfoWindow::Create(hWnd, m_pDevice, m_pContext));

		Game::Set_RenderCallback(&Render_ImGui);
	}
#endif

	return S_OK;
}

void CMainApp::Update(float DT)
{
#ifdef _DEBUG
	/* Clean */
	if (m_isOnImGui && m_pImGuiManager != nullptr)
		m_pImGuiManager->Clear_ImGuiFrame();
#endif

	Game::Update(DT);

#ifdef _DEBUG
	if (m_isOnImGui && m_pImGuiManager != nullptr)
		m_pImGuiManager->Update(0.f);
#endif
}

HRESULT CMainApp::Render()
{
	return Game::Render();
}

void CMainApp::Resize(uint32 iWinSizeX, uint32 iWinSizeY)
{
	if (m_pGameManager != nullptr)
		m_pGameManager->GraphicDevice_Resize(iWinSizeX, iWinSizeY);
}

CMainApp* CMainApp::Create(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI)
{
	CMainApp* pInstance = new CMainApp;

	if (FAILED(pInstance->Initialize(hWnd, hInst, iWinSizeX, iWinSizeY, eGraphicAPI)))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

#ifdef _DEBUG
	Game::Set_RenderCallback(nullptr);
	g_pImGuiManager = nullptr;
	if (m_pImGuiManager != nullptr)
		m_pImGuiManager->DestroyInstance();
	m_pImGuiManager = nullptr;
#endif

	m_pDevice.Reset();
	m_pContext.Reset();

	Game::Release();

	Safe_Release(m_pGameManager);
}

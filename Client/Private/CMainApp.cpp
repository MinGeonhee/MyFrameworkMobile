#include "CMainApp.h"

#include "CGameManager.h"

CMainApp::~CMainApp()
{
    Free();
}

HRESULT CMainApp::Initialize(HWND hWnd,
                             uint32 iWinSizeX,
                             uint32 iWinSizeY,
                             GRAPHIC_API eGraphicAPI)
{
    Engine_Desc EngineDesc{};
    EngineDesc.eGraphicAPI = eGraphicAPI;
    EngineDesc.pNativeWindow = hWnd;
    EngineDesc.iWinSizeX = iWinSizeX;
    EngineDesc.iWinSizeY = iWinSizeY;

    m_pGameManager = CGameManager::GetInstance();

    return m_pGameManager->Initialize_Engine(EngineDesc);
}

void CMainApp::Update()
{
    if (m_pGameManager == nullptr)
        return;

    m_pGameManager->TimerManager_Compute_TimeDelta(L"Timer_Default");
    const float fTimeDelta = m_pGameManager->TimerManager_Get_TimeDelta(L"Timer_Default");

    m_pGameManager->Pre_Update_Engine(fTimeDelta);
    m_pGameManager->Update_Engine(fTimeDelta);
    m_pGameManager->Late_Update_Engine(fTimeDelta);
}

void CMainApp::Render()
{
    if (m_pGameManager != nullptr)
        m_pGameManager->Draw();
}

void CMainApp::Resize(uint32 iWinSizeX, uint32 iWinSizeY)
{
    if (m_pGameManager != nullptr)
        m_pGameManager->Resize_Engine(iWinSizeX, iWinSizeY);
}

CMainApp* CMainApp::Create(HWND hWnd,
                           uint32 iWinSizeX,
                           uint32 iWinSizeY,
                           GRAPHIC_API eGraphicAPI)
{
    CMainApp* pMainApp = new CMainApp{};
    if (FAILED(pMainApp->Initialize(hWnd, iWinSizeX, iWinSizeY, eGraphicAPI)))
    {
        delete pMainApp;
        return nullptr;
    }
    return pMainApp;
}

void CMainApp::Free()
{
    if (m_pGameManager != nullptr)
    {
        m_pGameManager->Release_Engine();
        m_pGameManager = nullptr;
    }

    CBase::Free();
}

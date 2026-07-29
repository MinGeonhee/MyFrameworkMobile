#include "CMainApp.h"

#include "CGameManager.h"
#include "Game_Entry.h"

CMainApp::~CMainApp()
{
    Free();
}

HRESULT CMainApp::Initialize(HWND hWnd,
                             uint32 iWinSizeX,
                             uint32 iWinSizeY,
                             GRAPHIC_API eGraphicAPI)
{
    Engine::Engine_Desc EngineDesc{};
    EngineDesc.eGraphicAPI = eGraphicAPI;
    EngineDesc.pNativeWindow = hWnd;
    EngineDesc.iWinSizeX = iWinSizeX;
    EngineDesc.iWinSizeY = iWinSizeY;

    m_pGameManager = CGameManager::GetInstance();

    if (FAILED(m_pGameManager->Initialize_Engine(EngineDesc)))
        return E_FAIL;

    if(FAILED(m_pGameManager->SceneManager_Register_Factory(1,[](const Engine::CREATE_SCENE_DESC& Desc)->CScene*{return Game::Create_TestScene(Desc);})))
        return E_FAIL;
    Engine::CREATE_SCENE_DESC TestSceneDesc{};TestSceneDesc.iMakeSceneID=1;
    m_pGameManager->SceneManager_Request_Change(TestSceneDesc);

    return S_OK;
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


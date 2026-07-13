#include "CGameManager.h"

#include "CGraphicDevice.h"
#include "CTimeManger.h"

IMPLEMENT_SINGLETON(CGameManager)

CGameManager::CGameManager() = default;

HRESULT CGameManager::Initialize_Engine(const Engine_Desc& tEngineDesc)
{
    m_pTimerManager = CTimeManger::Create();
    if (m_pTimerManager == nullptr)
        return E_FAIL;

    if (FAILED(m_pTimerManager->Add_Timer(L"Timer_Default")))
        return E_FAIL;

    m_pGraphicDevice = new CGraphicDevice{};
    if (m_pGraphicDevice == nullptr)
        return E_FAIL;

    return m_pGraphicDevice->Initialize(tEngineDesc);
}

void CGameManager::Pre_Update_Engine(float fTimeDelta)
{
}

void CGameManager::Update_Engine(float fTimeDelta)
{
}

void CGameManager::Late_Update_Engine(float fTimeDelta)
{
}

void CGameManager::Draw()
{
    if (m_pGraphicDevice == nullptr)
        return;

    m_pGraphicDevice->Begin_Render();

    // RenderManager draw calls go here.

    m_pGraphicDevice->End_Render();
}

void CGameManager::Resize_Engine(uint32 iWinSizeX, uint32 iWinSizeY)
{
    if (m_pGraphicDevice != nullptr)
        m_pGraphicDevice->Resize(iWinSizeX, iWinSizeY);
}

void CGameManager::Release_Engine()
{
    Free();
    DestroyInstance();
}

HRESULT CGameManager::TimerManager_Add_Timer(const std::wstring& strKey)
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Add_Timer(strKey) : E_FAIL;
}

float CGameManager::TimerManager_Get_TimeDelta(const std::wstring& strKey) const
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Get_TimeDelta(strKey) : 0.f;
}

void CGameManager::TimerManager_Compute_TimeDelta(const std::wstring& strKey)
{
    if (m_pTimerManager != nullptr)
        m_pTimerManager->Compute_TimeDelta(strKey);
}

uint32 CGameManager::TimerManger_Get_FPS(const std::wstring& strKey) const
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Get_FPS(strKey) : 0;
}

void CGameManager::Free()
{
    delete m_pTimerManager;
    m_pTimerManager = nullptr;

    delete m_pGraphicDevice;
    m_pGraphicDevice = nullptr;

    CBase::Free();
}

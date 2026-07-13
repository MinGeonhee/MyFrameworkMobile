#include "CTimeManger.h"

#include "CTimer.h"

CTimeManger::~CTimeManger()
{
    Free();
}

HRESULT CTimeManger::Initialize()
{
    return S_OK;
}

HRESULT CTimeManger::Add_Timer(const std::wstring& strKey)
{
    if (Find_Timer(strKey) != nullptr)
        return E_FAIL;

    CTimer* pTimer = CTimer::Create();
    if (pTimer == nullptr)
        return E_FAIL;

    m_mapTimer.emplace(strKey, pTimer);
    return S_OK;
}

float CTimeManger::Get_TimeDelta(const std::wstring& strKey) const
{
    CTimer* pTimer = Find_Timer(strKey);
    return pTimer != nullptr ? pTimer->Get_TimeDelta() : 0.f;
}

uint32 CTimeManger::Get_FPS(const std::wstring& strKey) const
{
    CTimer* pTimer = Find_Timer(strKey);
    return pTimer != nullptr ? pTimer->Get_FPS() : 0;
}

void CTimeManger::Compute_TimeDelta(const std::wstring& strKey)
{
    CTimer* pTimer = Find_Timer(strKey);
    if (pTimer != nullptr)
        pTimer->Compute_TimeDelta();
}

CTimer* CTimeManger::Find_Timer(const std::wstring& strKey) const
{
    const auto Iter = m_mapTimer.find(strKey);
    return Iter != m_mapTimer.end() ? Iter->second : nullptr;
}

CTimeManger* CTimeManger::Create()
{
    CTimeManger* pTimeManager = new CTimeManger{};
    if (FAILED(pTimeManager->Initialize()))
    {
        delete pTimeManager;
        return nullptr;
    }
    return pTimeManager;
}

void CTimeManger::Free()
{
    for (auto& TimerPair : m_mapTimer)
        delete TimerPair.second;
    m_mapTimer.clear();

    CBase::Free();
}

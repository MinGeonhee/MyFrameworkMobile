#include "CTimer.h"

CTimer::CTimer() = default;

HRESULT CTimer::Initialize()
{
    m_LastTime = Clock::now();
    return S_OK;
}

void CTimer::Compute_TimeDelta()
{
    const auto CurrentTime = Clock::now();
    m_fTimeDelta = std::chrono::duration<float>(CurrentTime - m_LastTime).count();
    m_LastTime = CurrentTime;

    m_fFPSTime += m_fTimeDelta;
    ++m_iFrameCount;

    if (m_fFPSTime >= 1.f)
    {
        m_iFPS = m_iFrameCount;
        m_iFrameCount = 0;
        m_fFPSTime -= 1.f;
    }
}

CTimer* CTimer::Create()
{
    CTimer* pTimer = new CTimer{};
    if (FAILED(pTimer->Initialize()))
    {
        delete pTimer;
        return nullptr;
    }
    return pTimer;
}

void CTimer::Free()
{
    CBase::Free();
}

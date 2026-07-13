#pragma once

#include "CBase.h"

#include <chrono>

class CTimer final : public CBase
{
private:
    CTimer();

public:
    ~CTimer() override = default;

    float Get_TimeDelta() const { return m_fTimeDelta; }
    uint32 Get_FPS() const { return m_iFPS; }
    void Compute_TimeDelta();

private:
    HRESULT Initialize();

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point m_LastTime;
    float m_fFPSTime = 0.f;
    uint32 m_iFrameCount = 0;
    uint32 m_iFPS = 0;
    float m_fTimeDelta = 0.f;

public:
    static CTimer* Create();
    void Free() override;
};

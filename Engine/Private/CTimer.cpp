#include "CTimer.h"
#include "CTimeManger.h"

CTimer::CTimer()
	: m_fTimeDelta(0.f)
{
	InitMemory(m_CpuTick);
	InitMemory(m_FixTime);
	InitMemory(m_FrameTime);
	InitMemory(m_LastTime);
}

HRESULT CTimer::Initialize()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1077
	QueryPerformanceCounter(&m_LastTime);			// 1085
	QueryPerformanceCounter(&m_FixTime);			// 1090

	QueryPerformanceFrequency(&m_CpuTick);		// cpu tick 값을 얻어오는 함수

	return S_OK;
}

void CTimer::Compute_TimeDelta()
{
	QueryPerformanceCounter(&m_FrameTime);			// 1500

	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}


	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / Cast_float(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;


	// --- FPS 계산 ---
	m_fFPSTime += m_fTimeDelta;
	m_iFrameCount++;

	if (m_fFPSTime >= 1.0f)
	{
		m_iFPS = m_iFrameCount;
		m_iFrameCount = 0;
		m_fFPSTime = 0.f;
	}
}

CTimer* CTimer::Create()
{
	CTimer* pInstance = new CTimer;

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("CTimer is Failed To Create");
		Engine::Safe_Release(pInstance);
	}

	return pInstance;
}

void	CTimer::Free()
{
	__super::Free();

}

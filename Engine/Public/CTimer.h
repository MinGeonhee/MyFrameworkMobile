#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CTimer final :  public CBase
{
private:
	explicit CTimer();
	virtual	~CTimer() = default;
public:
	float	Get_TimeDelta() const { return m_fTimeDelta; }
	uint32	Get_FPS()		const { return m_iFPS; }
	void	Compute_TimeDelta();
private:
	HRESULT	Initialize();

private:
	LARGE_INTEGER	m_FrameTime;
	LARGE_INTEGER	m_FixTime;
	LARGE_INTEGER	m_LastTime;
	LARGE_INTEGER	m_CpuTick;


	float			m_fFPSTime = 0.f;
	uint32			m_iFrameCount = 0;
	uint32			m_iFPS = 0;
	float			m_fTimeDelta;
public:
	static	CTimer* Create();
	virtual void	Free();
};


NS_END


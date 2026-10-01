#pragma once
#include "CBase.h"

NS_BEGIN(Engine)	

class CTimer;

class CTimeManger : public CBase
{
private:
	explicit CTimeManger();
	virtual ~CTimeManger() =  default;

private:
	HRESULT	Initialize();
public:
	//타이머 추가
	HRESULT	Add_Timer(const wstring strKey);
	//원하는 Key값의 DT값 가져오기
	float	Get_TimeDelta(const wstring strKey);
	uint32  Get_FPS(const wstring strKey);
	void	Compute_TimeDelta(const wstring strKey);
private:
	CTimer* Find_Timer(const wstring strKey);
private:
	map<const wstring, CTimer*>	m_mapTimer;
public:
	static	CTimeManger* Create();
	virtual	void		 Free() override;
};

NS_END


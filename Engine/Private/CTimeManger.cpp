#include "CTimeManger.h"
#include "CTimer.h"


CTimeManger::CTimeManger()
{
	m_mapTimer.clear();
}

HRESULT	CTimeManger::Initialize()
{
	return S_OK;
}

HRESULT	CTimeManger::Add_Timer(const wstring strKey)
{
	CTimer* pTimer = { nullptr };

	pTimer = CTimeManger::Find_Timer(strKey);
	
	if (pTimer != nullptr)
		return E_FAIL;

	pTimer = CTimer::Create();
	if (pTimer == nullptr)
		return E_FAIL;

	m_mapTimer.emplace(strKey , pTimer);

	return S_OK;
}

float CTimeManger::Get_TimeDelta(const wstring strKey)
{
	CTimer* pTimer = CTimeManger::Find_Timer(strKey);

	if (pTimer == nullptr)
		return 0.f;

	return pTimer->Get_TimeDelta();
}

uint32 CTimeManger::Get_FPS(const wstring strKey)
{
	return m_mapTimer[strKey]->Get_FPS();
}

void	CTimeManger::Compute_TimeDelta(const wstring strKey)
{
	CTimer* pTimer = Find_Timer(strKey);
	if (nullptr == pTimer)
		return;

	pTimer->Compute_TimeDelta();
}
CTimer* CTimeManger::Find_Timer(const wstring strKey)
{
	auto iter = m_mapTimer.find(strKey);

	if (iter == m_mapTimer.end())
		return nullptr;

	return iter->second;
}

CTimeManger* CTimeManger::Create()
{
	CTimeManger* pTimeManager = new CTimeManger;

	INIT_FAILED_CEHCK(pTimeManager);

	return pTimeManager;
}

void CTimeManger::Free()
{
	__super::Free();

	for (auto TImer_Pair : m_mapTimer)
		Safe_Release(TImer_Pair.second);
	m_mapTimer.clear();
}
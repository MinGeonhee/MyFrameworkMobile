#include "CScreenShotManager.h"
#include "CScreenShot.h"
#include "CGameManager.h"
#include "CUI_Quad.h"

#include <wincodec.h> 


CScreenShotManager::CScreenShotManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext), m_mapScreenShot() ,  m_pOverLayScreenShot{nullptr}
	, m_queueScreenShotFunc{}
{
}

HRESULT	CScreenShotManager::Initialize()
{
	return S_OK;
}


HRESULT CScreenShotManager::Add_ScreenShot(const wstring& strKey, CScreenShot* pScreenShot)
{
	CScreenShot* pFindScreenShot = Find_ScreenShot(strKey);


	if (pFindScreenShot != nullptr)
		return E_FAIL;
	
	if (strKey.empty())
	{
		/* 현재 날짜 가져오는거 없나... */
		SYSTEMTIME st;
		GetLocalTime(&st);
		wstring wstrTime = std::to_wstring(st.wYear) + L"-"+ std::to_wstring(st.wMonth)+ L"-" + std::to_wstring(st.wDay) + L"-" + std::to_wstring(st.wMilliseconds);
		m_mapScreenShot.emplace(wstrTime, pScreenShot);

		return S_OK;
	}

	m_mapScreenShot.emplace( strKey , pScreenShot);

	return S_OK;
}

CScreenShot* CScreenShotManager::Get_ScreenShot(const wstring& strKey)
{
	return Find_ScreenShot(strKey);
}


HRESULT CScreenShotManager::ScreenShot(bool isOverLay, _Outptr_ CScreenShot** ppScreenShot)
{

	ComPtr<Texture2D> pBackBuffer{ nullptr };

	if (FAILED(CGameManager::GetInstance()->GraphicDevice_Get_BackBuffer(&pBackBuffer)))
		return E_FAIL;

	CScreenShot* pScreenShot = CScreenShot::Create(m_pDevice, m_pDeviceContext, pBackBuffer);
	NULLPTR_RETURN_EFAIL(pScreenShot);

	if (isOverLay == true)
	{
		/* 기존에있던 오버레이를 지운다 */
		Safe_Release(m_pOverLayScreenShot);
		m_pOverLayScreenShot = pScreenShot;
	}
	else
		Add_ScreenShot(L"", pScreenShot);


	if (ppScreenShot != nullptr)
		*ppScreenShot = pScreenShot;

	return S_OK;
}

void CScreenShotManager::Request_ScreenShot(bool isOverLay, _Outptr_ CScreenShot** ppScreenShot)
{ 
	m_queueScreenShotFunc.push([=](){return ScreenShot(isOverLay,ppScreenShot);});
}

HRESULT CScreenShotManager::ExecutePending_ScreenShot()
{
	if (m_queueScreenShotFunc.empty())
		return S_OK;


	if (FAILED(m_queueScreenShotFunc.front()()))
		return E_FAIL;

	m_queueScreenShotFunc.pop();

	return S_OK;
}


CScreenShot* CScreenShotManager::Find_ScreenShot(const wstring& strKey)
{
	auto iter = m_mapScreenShot.find(strKey);

	if (iter == m_mapScreenShot.end())
		return nullptr;

	return  iter->second;
}

CScreenShotManager* CScreenShotManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CScreenShotManager* pStateManager = new CScreenShotManager(pDevice, pContext);

	INIT_FAILED_CEHCK(pStateManager);

	return pStateManager;
}

void CScreenShotManager::Free()
{
	__super::Free();

	/*CScreenShotManager::Free()*/

	for (auto TextureMap : m_mapScreenShot)
		Safe_Release(TextureMap.second);

	Safe_Release(m_pOverLayScreenShot);
}

HRESULT ScreenShot(bool isOverLay)
{
	return E_NOTIMPL;
}

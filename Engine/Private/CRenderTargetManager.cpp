#include "CRenderManager.h"
#include "CRenderTargetManager.h"
#include "CRenderTarget.h"
#include "CGraphicDevice.h"

CRenderTargetManager::CRenderTargetManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext), m_umapRenderTarget{}, m_umapMRT{}
	, m_pPreDSV(nullptr), m_pBackBuffer{nullptr}
{
}

HRESULT CRenderTargetManager::Add_RenderTarget(const wstring& wstrAddKey, uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(wstrAddKey);

	if (pRenderTarget != nullptr) return E_FAIL;


	pRenderTarget = CRenderTarget::Create(iWidth,iHeight,vColor,eFormat,m_pDevice,m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(pRenderTarget);
	pRenderTarget->Clear_RenderTargetView();
	m_umapRenderTarget.emplace(wstrAddKey, pRenderTarget);

	return S_OK;
}

HRESULT CRenderTargetManager::Add_MRT(const wstring& wstrAddMRTKey, const wstring& wstrFindRenderTargetKey)
{
	/* RenderTarget 부터 찾아주자 */
	CRenderTarget* pRenderTarget = Find_RenderTarget(wstrFindRenderTargetKey);

	NULLPTR_RETURN_EFAIL(pRenderTarget);

	m_umapMRT[wstrAddMRTKey].push_back(pRenderTarget);

	Safe_AddRef(pRenderTarget);

	return S_OK;
}

ShaderResourceView* CRenderTargetManager::Get_ShaderResourceView(const wstring& wstrFindRenderTargetKey) 
{
	const CRenderTarget* pFindRenderTarget = Find_RenderTarget(wstrFindRenderTargetKey);

	NULLPTR_RETURN_NULLPTR(pFindRenderTarget);

	return pFindRenderTarget->Get_SRV();
}

HRESULT CRenderTargetManager::Begin_MRT(const wstring& wstrFindMRT)
{
	list<CRenderTarget*>* pRenderTargetList = Find_MRT(wstrFindMRT);
	NULLPTR_RETURN_EFAIL(pRenderTargetList);

	/* 원래 이전의 RTV , DSV 를 잠시 보관해준다 */
	/* OMGetRenderTarget으로 가져오게되면 자동으로 refernece Counter가 증가되므로 나중에 돌려줄대 Safe_Release를 반드시 불러주자 */
	CGraphicDevice::Get_Instance()->OM_GetRenderTargets(1, &m_pBackBuffer, &m_pPreDSV);

	/* 이 이후 장치에 세팅 */

	RenderTargetView* pRTV[8] = {nullptr};

	uint32 iCount = 0;
	for (auto& RenderTarget : *pRenderTargetList)
	{
		/* Render 부르기전에 RenderTargetView Clear */
		RenderTarget->Clear_RenderTargetView();
		pRTV[iCount++] = RenderTarget->Get_RTV();
	}

	/* RenderTarget View Setting */
	CGraphicDevice::Get_Instance()->OM_SetRenderTargets(iCount, pRTV, m_pPreDSV);

	return S_OK;
}

HRESULT CRenderTargetManager::Begin_MRT_NoClear(const wstring& wstrFindMRT)
{
	list<CRenderTarget*>* pRenderTargetList = Find_MRT(wstrFindMRT);
	NULLPTR_RETURN_EFAIL(pRenderTargetList);

	CGraphicDevice::Get_Instance()->OM_GetRenderTargets(1, &m_pBackBuffer, &m_pPreDSV);

	RenderTargetView* pRTV[8] = { nullptr };

	uint32 iCount = 0;
	for (auto& RenderTarget : *pRenderTargetList)
		pRTV[iCount++] = RenderTarget->Get_RTV();

	CGraphicDevice::Get_Instance()->OM_SetRenderTargets(iCount, pRTV, m_pPreDSV);

	return S_OK;
}

HRESULT CRenderTargetManager::Clear_MRT_Rect(const wstring& wstrFindMRT, const RECT_DESC& tRect)
{
	list<CRenderTarget*>* pRenderTargetList = Find_MRT(wstrFindMRT);
	NULLPTR_RETURN_EFAIL(pRenderTargetList);

	/* 원본 : ID3D11DeviceContext1::ClearView (영역 Clear) */
	for (auto& RenderTarget : *pRenderTargetList)
		CGraphicDevice::Get_Instance()->Clear_RenderTarget_Rect(RenderTarget->Get_RTV(), RenderTarget->Get_ClearColor(), tRect);

	return S_OK;
}

HRESULT CRenderTargetManager::Bind_ShaderResource(const wstring& wstrFindRenderTargetKey, const string& strBindTexName, CShader* pShader)
{
	CRenderTarget* pFindRenderTarget = Find_RenderTarget(wstrFindRenderTargetKey);
	return pFindRenderTarget->Bind_ShaderResource(strBindTexName,pShader);
}

HRESULT CRenderTargetManager::End_MRT()
{
	/* 원래이전의 Pre RTV , DSV 세팅 */
	RenderTargetView* pRenderTargets[8] = {
	m_pBackBuffer
	};


	CGraphicDevice::Get_Instance()->OM_SetRenderTargets(8, pRenderTargets, m_pPreDSV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pPreDSV);

	return S_OK;
}

#ifdef _DEBUG
HRESULT CRenderTargetManager::Ready_Debug(const wstring& wstrFindRenderTargetKey, float fX, float fY, float fSizeX, float fSizeY)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(wstrFindRenderTargetKey);
	NULLPTR_RETURN_EFAIL(pRenderTarget);

	pRenderTarget->Ready_Debug(fX, fY, fSizeX, fSizeY);

	return S_OK;
}

HRESULT CRenderTargetManager::Render_Debug(const wstring& wstrFindRenderTargtKey, CShader* pShader, CVIBuffer_Rect* pBuffer)
{
	list<CRenderTarget*>* pRenderTargetList = Find_MRT(wstrFindRenderTargtKey);
	NULLPTR_RETURN_EFAIL(pRenderTargetList);

	for (auto& RenderTarget : *pRenderTargetList)
		if (RenderTarget)
			if(FAILED(RenderTarget->Render(pShader,pBuffer)))
				return E_FAIL;

	return S_OK;
}
#endif


CRenderTarget* CRenderTargetManager::Find_RenderTarget(const wstring& wstrFindKey)
{
	const auto& Iter = m_umapRenderTarget.find(wstrFindKey);

	if (Iter == m_umapRenderTarget.end()) return nullptr;

	return Iter->second;
}

list<CRenderTarget*>* CRenderTargetManager::Find_MRT(const wstring& wstrFindKey)
{
	const auto& Iter = m_umapMRT.find(wstrFindKey);

	if ( Iter == m_umapMRT.end()) return nullptr;

	return &Iter->second;
}

HRESULT CRenderTargetManager::Initialize()
{
	return S_OK;
}


CRenderTargetManager* CRenderTargetManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CRenderTargetManager* pMgr = new CRenderTargetManager(pDevice, pContext);

	if (FAILED(pMgr->Initialize()))
	{
		Safe_Release(pMgr);
		MSG_BOX(" Render Target Manager is failed to Create ");
		return nullptr;
	}

	return pMgr;
}

void CRenderTargetManager::Free()
{
	__super::Free();

	for (auto& RTVs : m_umapMRT)
	{
		for (auto& RTV : RTVs.second)
			Safe_Release(RTV);
		RTVs.second.clear();
	}
	m_umapMRT.clear();



	for (auto& RenderTarget : m_umapRenderTarget)
		Safe_Release(RenderTarget.second);
	m_umapRenderTarget.clear();

}

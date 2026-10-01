#include "CUIManager.h"
#include "CGameManager.h"
#include "CBlendState.h"
#include "CDepthstencilState.h"
#include "CCanvas.h"
#include "CUILayer.h"
#include "CScreenFadeUI.h"

CUIManager::CUIManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext), m_mapCanvas{} , m_pBlendState(nullptr), m_pDSV(nullptr) , m_pScreenFadeUI(nullptr)
	, m_pGameManager(CGameManager::GetInstance()) , m_pDefaultCanvas(nullptr)
{
	Safe_AddRef(m_pGameManager);
}


HRESULT	CUIManager::Initialize()
{
	/* Setting 해줄 State 생성 */

	if (FAILED(Ready_RenderState()))
		return E_FAIL;

	
	return S_OK;
}

HRESULT CUIManager::Ready_DefaultCanvas()
{
	float SizeX = static_cast<float>(m_pGameManager->Get_DefaultWinSizeX());
	float SizeY = static_cast<float>(m_pGameManager->Get_DefaultWinSizeY());

	CCanvas::Canvas_DESC tDesc{};
	tDesc.vCanvasSize = { SizeX , SizeY };
	tDesc.vCanvasStartPos = {0.f,0.f};
	tDesc.wstrShaderKey = Shader_EngineUI;

	m_pDefaultCanvas = CCanvas::Create(&tDesc, m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pDefaultCanvas);

	CUILayer* pLayer = CUILayer::Create(UILAYER_ORDER::ORDER_0);
	NULLPTR_RETURN_EFAIL(pLayer);
	m_pDefaultCanvas->Add_UILayer(L"Default_Layer",pLayer);

	m_pScreenFadeUI = CScreenFadeUI::Create(m_pDevice, m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pScreenFadeUI);
	pLayer->Add_UIObject(m_pScreenFadeUI);

	/* 특별한 이벤트발동시에 업데이트진행 */
	m_pDefaultCanvas->Set_IsActive(false);

	return S_OK;
}

HRESULT	CUIManager::Ready_RenderState()
{
	BLEND_DESC tUIBlendDesc{};
#pragma region BlendState_Desc_Blend
	tUIBlendDesc.AlphaToCoverageEnable = false;
	tUIBlendDesc.IndependentBlendEnable = false;

	for (uint32 i = 0; i < 8; ++i)
	{
		Diligent::RenderTargetBlendDesc& tRT = tUIBlendDesc.RenderTargets[i];
		tRT.BlendEnable = true;
		tRT.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
		tRT.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		tRT.BlendOp = Diligent::BLEND_OPERATION_ADD;
		tRT.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
		tRT.DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
		tRT.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
		tRT.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;
	}
#pragma endregion
	DEPTH_STENCIL_DESC   tDepthStencilState_Desc_NoDepth{};
	tDepthStencilState_Desc_NoDepth.DepthEnable = false;
	tDepthStencilState_Desc_NoDepth.DepthWriteEnable = false;

	m_pBlendState = CBlendState::Create(m_pDevice, m_pDeviceContext, tUIBlendDesc);
	m_pDSV = CDepthStencilState::Create(m_pDevice, m_pDeviceContext, tDepthStencilState_Desc_NoDepth);
	NULLPTR_RETURN_EFAIL(m_pBlendState);
	NULLPTR_RETURN_EFAIL(m_pDSV);

	return S_OK;
}


void	CUIManager::Fade_Out(float fTime)
{
	if (!m_pDefaultCanvas) return;
	if (!m_pScreenFadeUI) return;


	m_pDefaultCanvas->Set_IsActive(true);
	m_pScreenFadeUI->Set_IsActive(true);
	m_pScreenFadeUI->Fade_Out(fTime);
}

void	CUIManager::Fade_In(float fTime)
{
	if (!m_pDefaultCanvas) return;
	if (!m_pScreenFadeUI) return;


	m_pDefaultCanvas->Set_IsActive(true);
	m_pScreenFadeUI->Set_IsActive(true);
	m_pScreenFadeUI->Fade_In(fTime);
}



HRESULT CUIManager::Add_Canvas(uint32 iIndex,CCanvas* pCanvas)
{
	auto* pFind = Get_Canvas(iIndex);
	if (pFind != nullptr)  return E_FAIL;

	m_mapCanvas.emplace(iIndex,pCanvas);

	return S_OK;
}

CCanvas* CUIManager::Get_Canvas(uint32 iIndex)
{
	auto iter = m_mapCanvas.find(iIndex);
	if (iter == m_mapCanvas.end()) return nullptr;

	return iter->second;
}

void CUIManager::Release_Canvas(uint32 iClearIndex)
{
	auto* FindCanvas = Get_Canvas(iClearIndex);
	Safe_Release(FindCanvas);
	m_mapCanvas.erase(iClearIndex);
	return;
}

void CUIManager::Pre_Update(float DT)
{
	for (auto& Canvas : m_mapCanvas)
		if (Canvas.second)
			if (Canvas.second->Get_IsActive())
				Canvas.second->Pre_Update(DT);


	if (m_pDefaultCanvas->Get_IsActive())
		m_pDefaultCanvas->Pre_Update(DT);
}

void CUIManager::Update(float DT)
{

	for (auto& Canvas : m_mapCanvas)
		if (Canvas.second)
			if (Canvas.second->Get_IsActive())
				Canvas.second->Update(DT);

	if(m_pScreenFadeUI->Get_IsActive())
		m_pScreenFadeUI->Update(nullptr,DT);


	if(m_pDefaultCanvas->Get_IsActive())
		m_pDefaultCanvas->Update(DT);
}

void CUIManager::Late_Update(float DT)
{

	for (auto& Canvas : m_mapCanvas)
		if (Canvas.second)
			if(Canvas.second->Get_IsActive())
				Canvas.second->Late_Update(DT);


	if (m_pDefaultCanvas->Get_IsActive())
		m_pDefaultCanvas->Late_Update(DT);
}

HRESULT	CUIManager::Render()
{
	/* UI Render전에 반드시 깊이비교를 꺼줘야함 */
	m_pBlendState->Set_GPU_BlendState();
	m_pDSV->Set_GPU_DepthstencilState();

	m_pGameManager->GraphicDevice_Set_GPU_OriginViewPort();


	vector<CUILayer*> vecUILayer{};

	for (auto iter = m_mapCanvas.rbegin(); iter != m_mapCanvas.rend(); ++iter)
	{
		auto& pCanvas = iter->second;

		if (pCanvas)
		{
			if (pCanvas->Get_IsActive())
			{
				if (FAILED(pCanvas->Render()))
					return E_FAIL;
			}
		}
	}

	if (m_pDefaultCanvas->Get_IsActive())
		m_pDefaultCanvas->Render();

	return S_OK;
}

HRESULT CUIManager::Ready_DefaultUI()
{
	if (FAILED(Ready_DefaultCanvas()))
		return E_FAIL;

	return S_OK;
}


CUIManager* CUIManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CUIManager* pUIManager = new CUIManager(pDevice, pContext);

	if (FAILED(pUIManager->Initialize()))
	{
		Safe_Release(pUIManager);
		MSG_BOX("UIManger is failed to Create");
		return nullptr;
	}
	return pUIManager;
}

void CUIManager::Free()
{
	__super::Free();
	for (auto& Canvas : m_mapCanvas)
		Safe_Release(Canvas.second);




	Safe_Release(m_pBlendState);
	Safe_Release(m_pDSV);
	Safe_Release(m_pDefaultCanvas);
	Safe_Release(m_pGameManager);

	return;
}

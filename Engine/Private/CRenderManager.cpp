#include "CRenderManager.h"
#include "CLayer.h"
#include "CGameObject.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CTransform.h"
#include "CRasterizerState.h"
#include "CBlendState.h"
#include "CDepthstencilState.h"
#include "CUIObject.h"
#include "CCameraObjectBase.h"
#include "CVIBuffer_Rect.h"
#include "CGraphicDevice.h"

#include "Graphics/GraphicsEngine/interface/Query.h"


CRenderManager::CRenderManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: m_pDevice(pDevice), m_pDeviceContext(pContext), m_RenderGroups{},m_pGameManager(CGameManager::GetInstance())
	, m_matWorld{}, m_matProj{}, m_matView{}, m_matGlobalInvProj{} , m_pBuffer(nullptr), m_pShader(nullptr)
	, m_eRenderMode(RENDER_MODE::PERCAM), m_isPerCameraDeferred(true), m_tGpuTimer{}, m_iGpuTimerIndex(0), m_iGpuSampleCount(0), m_dGpuTimeSum(0.0)
#ifdef _DEBUG
	, m_vecDebugComponent{}
#endif
{
	Safe_AddRef(m_pGameManager);

	XMStoreFloat4x4(&m_matWorld,XMMatrixIdentity());
	XMStoreFloat4x4(&m_matView,XMMatrixIdentity());
	XMStoreFloat4x4(&m_matProj,XMMatrixIdentity());
	XMStoreFloat4x4(&m_matGlobalInvProj,XMMatrixIdentity());
}

HRESULT CRenderManager::Initialize()
{	
	/* 미리 메모리공간 확보해놓기 */
	for (auto& Renders : m_RenderGroups)
		Renders.reserve(100);

	if (FAILED(Ready_GpuTimer()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderManager::Ready_Deferred()
{

	/* ViewPort */
	VIEWPORT_DESC tOriginViewPort = m_pGameManager->GraphicDevice_Get_OriginViewPort();


	/* RenderTarget Create */


	/* 객체마다 랜더타겟을이용하여 픽셀에 저장할 정보들의 대한 랜더타겟 */

	/* Diffuse RenderTarget Create */
	if (FAILED(m_pGameManager->
		RenderTargetManager_Add_RenderTarget(
			L"Target_Diffuse", Cast_Uint(tOriginViewPort.Width), Cast_Uint(tOriginViewPort.Height), Color(0.f, 0.f, 0.f, 0.f), Diligent::TEX_FORMAT_RGBA8_UNORM)))
		return E_FAIL;

	/* Normal RenderTarget Create */
	if (FAILED(m_pGameManager->
		RenderTargetManager_Add_RenderTarget(
			L"Target_Normal", Cast_Uint(tOriginViewPort.Width), Cast_Uint(tOriginViewPort.Height), Color(0.f, 0.f, 0.f, 0.f), Diligent::TEX_FORMAT_RGBA16_UNORM)))
		return E_FAIL;

	/* Depth RenderTarget Create */
	if (FAILED(m_pGameManager->RenderTargetManager_Add_RenderTarget(
		L"Target_Depth", Cast_Uint(tOriginViewPort.Width), Cast_Uint(tOriginViewPort.Height), Color(0.f, 0.f, 0.f, 0.f), Diligent::TEX_FORMAT_RGBA32_FLOAT
	)))
		return E_FAIL;

	/* 위에서 계산된 픽셀들을 합쳐서 연산에 들어갈 랜더타겟 */

	/* Shade RenderTarget Create */
	if (FAILED(m_pGameManager->
		RenderTargetManager_Add_RenderTarget(
			L"Target_Shade", Cast_Uint(tOriginViewPort.Width), Cast_Uint(tOriginViewPort.Height), Color(0.f, 0.f, 0.f, 0.f), Diligent::TEX_FORMAT_RGBA16_UNORM)))
		return E_FAIL;

	/* Acc로 기존값과 섞어주는역할을 해서 Blend옵션을 켜줘야하기 떄문에 알파값이 0인게 유리하다 */
	if (FAILED(m_pGameManager->
		RenderTargetManager_Add_RenderTarget(
			L"Target_Specular", Cast_Uint(tOriginViewPort.Width), Cast_Uint(tOriginViewPort.Height), Color(0.f, 0.f,0.f, 0.f), Diligent::TEX_FORMAT_RGBA16_UNORM)))
		return E_FAIL;



	/* MRT Setting */

	/* Register MRT_GameObject */
	if (FAILED(m_pGameManager->RenderTargetManager_Add_MRT(L"MRT_GameObject", L"Target_Diffuse")))	return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Add_MRT(L"MRT_GameObject", L"Target_Normal")))	return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Add_MRT(L"MRT_GameObject" , L"Target_Depth")))	return E_FAIL;

	/* Register MRT_LightDeffered */
	if (FAILED(m_pGameManager->RenderTargetManager_Add_MRT(L"MRT_LightAcc", L"Target_Shade")))		return E_FAIL;
	/* Register MRT_LightDeffered */
	if (FAILED(m_pGameManager->RenderTargetManager_Add_MRT(L"MRT_LightAcc", L"Target_Specular")))	return E_FAIL;

	/* View Proj Setting */
	XMStoreFloat4x4(&m_matWorld, XMMatrixScaling(tOriginViewPort.Width, tOriginViewPort.Height, 1.f));
	m_matWorld._43 = 0.5f;
	XMStoreFloat4x4(&m_matView,XMMatrixIdentity());
	XMStoreFloat4x4(&m_matProj,XMMatrixOrthographicLH(tOriginViewPort.Width, tOriginViewPort.Height,0.f,1.f));

	Matrix ProjMatrix = XMMatrixPerspectiveFovLH(60, tOriginViewPort.Width / (float)tOriginViewPort.Height, 0.1f, 500.f);
	XMStoreFloat4x4(&m_matGlobalInvProj,ProjMatrix);



	m_pBuffer = CVIBuffer_Rect::Create(m_pDevice,m_pDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pBuffer);

	/* Shader를 미리 만들어야한다... */
	m_pShader = static_cast<CShader*>(m_pGameManager->ResourceManager_Get_Resource(Shader_Deferred , RESOURCE_TYPE::SHADER , false ));
	NULLPTR_RETURN_EFAIL(m_pShader);



#ifdef _DEBUG
	m_pGameManager->RenderTargetManager_Ready_Debug(L"Target_Diffuse"	,1280 * 0.5f ,720.f * 0.5f - (150.f * 2.f)	, 200.f , 150.f);
	m_pGameManager->RenderTargetManager_Ready_Debug(L"Target_Normal"	,1280 * 0.5f ,720.f * 0.5f - (150.f * 1.f)	, 200.f , 150.f);
	m_pGameManager->RenderTargetManager_Ready_Debug(L"Target_Depth"		,1280 * 0.5f ,720.f * 0.5f - (150.f * 0.f)	, 200.f , 150.f);
	m_pGameManager->RenderTargetManager_Ready_Debug(L"Target_Shade"		,1280 * 0.5f , 720.f * 0.5f - (150.f * -1.f)	, 200.f	, 150.f);
	m_pGameManager->RenderTargetManager_Ready_Debug(L"Target_Specular"  ,1280 * 0.5f , 720.f * 0.5f - (150.f * -2.f)	, 200.f	, 150.f);
#endif

	return S_OK;
}

HRESULT CRenderManager::Ready_RasterizerState()
{
	/* Fill Solid */
	RASTERIZER_DESC tRasterizerDesc{};
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"SOLID_CULL_BACK", CRasterizerState::Create(tRasterizerDesc,m_pDevice,m_pDeviceContext));
	tRasterizerDesc.CullMode = Diligent::CULL_MODE_FRONT;
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"SOLID_CULL_FRONT", CRasterizerState::Create(tRasterizerDesc, m_pDevice, m_pDeviceContext));
	tRasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"SOLID_CULL_NONE", CRasterizerState::Create(tRasterizerDesc, m_pDevice, m_pDeviceContext));


	/* Wire Frame */
	tRasterizerDesc.FillMode = Diligent::FILL_MODE_WIREFRAME;
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"WIREFRAME_CULL_NONE", CRasterizerState::Create(tRasterizerDesc, m_pDevice, m_pDeviceContext));
	tRasterizerDesc.CullMode = Diligent::CULL_MODE_FRONT;
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"WIREFRAME_CULL_FRONT", CRasterizerState::Create(tRasterizerDesc, m_pDevice, m_pDeviceContext));
	tRasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
	m_umapRenderState[Cast_Uint(STATE_TYPE::RASTERIZER)].emplace(L"WIREFRAME_CULL_BACK", CRasterizerState::Create(tRasterizerDesc, m_pDevice, m_pDeviceContext));



	return S_OK;
}

HRESULT CRenderManager::Ready_BlendState()
{
	BLEND_DESC tBlendStateDesc{};
	m_umapRenderState[Cast_Uint(STATE_TYPE::BLEND)].emplace(L"NONE_ALPHA", CBlendState::Create(m_pDevice, m_pDeviceContext,tBlendStateDesc));


	return S_OK;
}

HRESULT CRenderManager::Ready_DepthStencilState()
{
	return S_OK;
}


HRESULT CRenderManager::Ready_Default()
{
	if (FAILED(Ready_Deferred()))
		return E_FAIL;

	return S_OK;
}


HRESULT CRenderManager::RenderGruop(const vector<CCameraObjectBase*>& vecRenderCam)
{
	if (vecRenderCam.empty()) return S_OK;

	if (FAILED(Render_Priority(vecRenderCam)))
		return E_FAIL;

	if (FAILED(Render_NonBlend(vecRenderCam)))
		return E_FAIL;

	if (FAILED(Render_Light(vecRenderCam)))
		return E_FAIL;

	if (FAILED(Render_Combind(nullptr)))
		return E_FAIL;

	if (FAILED(Render_NoneLight(vecRenderCam)))
		return E_FAIL;

	if (FAILED(Render_Blend(vecRenderCam)))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(Render_Debug(vecRenderCam)))
		return E_FAIL;
#endif 

	return S_OK;
}

/* PERCAM : one camera == one full deferred sequence */
HRESULT CRenderManager::RenderGruop(CCameraObjectBase* pRenderCam)
{
	NULLPTR_RETURN_EFAIL(pRenderCam);

	const vector<CCameraObjectBase*> vecOneCam{ pRenderCam };

	if (FAILED(Render_Priority(vecOneCam)))			return E_FAIL;
	if (FAILED(Render_NonBlend(vecOneCam)))			return E_FAIL;
	if (FAILED(Render_Light(vecOneCam)))			return E_FAIL;
	if (FAILED(Render_Combind(pRenderCam)))			return E_FAIL;
	if (FAILED(Render_NoneLight(vecOneCam)))		return E_FAIL;
	if (FAILED(Render_Blend(vecOneCam)))			return E_FAIL;

	return S_OK;
}



HRESULT CRenderManager::Add_RenderGroup(RENDER_GROUP eRenderGruop, CGameObject* pGameObject)
{
	/* Add Render Group */
	m_RenderGroups[Cast_Uint(eRenderGruop)].push_back(pGameObject);
	/* Add Reference */
	Safe_AddRef(pGameObject);

	return S_OK;
}

HRESULT CRenderManager::Clean_RenderGroup()
{
	for (auto& RenderGroup : m_RenderGroups)
	{
		for (auto& RenderObject : RenderGroup)
		{
			Safe_Release(RenderObject);
		}
		RenderGroup.clear();
	}

	return S_OK;
}

HRESULT CRenderManager::Render_Priority(const vector<CCameraObjectBase*>& vecRenderCam)
{
	/* Sky Cube 그린뒤 세팅 */
	for (auto& RenderCam : vecRenderCam)
	{
		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();

		Float4 CamAreaXY = RenderCam->Get_CameraRenderAreaXY_MinMax();

		for (auto& RenderObject : m_RenderGroups[Cast_Uint(RENDER_GROUP::PRIORITY)])
		{
			if (!RenderObject || !RenderObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))
				continue;
			/* 컬링이 On이 아니거나 On이지만 Area 안에들어와 있지 않는 애들은 걸러진다 */
			if (RenderObject->Get_IsRenderCulling() && !RenderObject->Get_IsInCamArea(CamAreaXY))
				continue;
			if (FAILED(RenderObject->Render(RenderCam)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CRenderManager::Render_NonBlend(const vector<CCameraObjectBase*>& vecRenderCam)
{
	m_pGameManager->RenderTargetManager_Begin_MRT(L"MRT_GameObject");

	/* PERCAM : cameras may overlap, so each starts from a clean depth buffer */
	if (m_isPerCameraDeferred)
	{
		DepthStencilView* pDSV = nullptr;
		CGraphicDevice::Get_Instance()->OM_GetRenderTargets(0, nullptr, &pDSV);
		if (pDSV)
		{
			m_pDeviceContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			pDSV->Release();
		}
	}

	for (auto& RenderCam : vecRenderCam)
	{
		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();

		Float4 CamAreaXY = RenderCam->Get_CameraRenderAreaXY_MinMax();

		for (auto& RenderObject : m_RenderGroups[Cast_Uint(RENDER_GROUP::NONBLEND)])
		{
			if (!RenderObject || !RenderObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))		continue;
			if (RenderObject->Get_IsRenderCulling() && !RenderObject->Get_IsInCamArea(CamAreaXY))	continue;
			if (FAILED(RenderObject->Render(RenderCam)))											return E_FAIL;
		}

	}
	m_pGameManager->RenderTargetManager_End_MRT();


	return S_OK;
}

HRESULT CRenderManager::Render_Light(const vector<CCameraObjectBase*>& vecRenderCam)
{

	if (FAILED(m_pGameManager->RenderTargetManager_Begin_MRT(L"MRT_LightAcc")))
		return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Normal", "g_NormalTexture", m_pShader)))
		return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Depth", "g_DepthTexture", m_pShader)))
		return E_FAIL;


	m_pGameManager->GraphicDevice_Set_GPU_OriginViewPort();
	m_pShader->Set_GPU_Transform(m_matWorld);
	m_pShader->Set_GPU_Canvas(m_matProj);

	m_pShader->Set_GPU_Matrix("g_GlobalInvProjMatrix" , m_matGlobalInvProj );

	/* NormalTexture Binding */
	for (auto& RenderCam : vecRenderCam)
	{

		if (!RenderCam)
			continue;

		/* 카메라가가진 뷰포트영역만 해당하는 픽셀 좌표만 계산이된다 */
		RenderCam->Set_GPU_Camera();
		RenderCam->Set_ScissorRects();
		m_pGameManager->LightManager_Render_Light(m_pShader,m_pBuffer, RenderCam);
	}

	CGraphicDevice::Get_Instance()->RS_SetScissorRects(0, nullptr);


	if (FAILED(m_pGameManager->RenderTargetManager_End_MRT()))
		return E_FAIL;


	return S_OK;
}

HRESULT CRenderManager::Render_Combind(CCameraObjectBase* pRenderCam)
{
	m_pGameManager->GraphicDevice_Set_GPU_OriginViewPort();

	/* nullptr -> whole screen (SHARED). Otherwise only this camera's rect. */
	if (pRenderCam != nullptr)
		pRenderCam->Set_ScissorRects();
	else
		m_pGameManager->CGraphicDevice_Set_ScissorRectsOrigin();

	m_pShader->Set_GPU_Transform(m_matWorld);
	m_pShader->Set_GPU_Canvas(m_matProj);



	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Diffuse", "g_DiffuseTexture", m_pShader)))
		return S_OK;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Shade", "g_ShadeTexture", m_pShader)))
		return S_OK;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Specular","g_SpecularTexture", m_pShader)))
		return S_OK;



	m_pShader->Apply_Pass("Combined");
	m_pBuffer->Bind_Resource();
	m_pBuffer->Draw();

	return S_OK;
}

HRESULT CRenderManager::Render_NoneLight(const vector<CCameraObjectBase*>& vecRenderCam)
{
	for (auto& RenderCam : vecRenderCam) 
	{
		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();


		Float4 CamAreaXY = RenderCam->Get_CameraRenderAreaXY_MinMax();

		for (auto& RenderObject : m_RenderGroups[Cast_Uint(RENDER_GROUP::NONLIGHT)])
		{
			if (!RenderObject || !RenderObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))
				continue;
			if (RenderObject->Get_IsRenderCulling() && !RenderObject->Get_IsInCamArea(CamAreaXY))
				continue;
			if (FAILED(RenderObject->Render(RenderCam)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CRenderManager::Render_Blend(const vector<CCameraObjectBase*>& vecRenderCam)
{
	for (auto& RenderCam : vecRenderCam)
	{
		/* ViewPort Set && CamBuffer Setting */
		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();

		float CamZ = RenderCam->Get_Z();

		/* 알파 블렌딩이 필요하기 때문에 카메라 위치기준 Sort를 해준다 */
		std::sort(m_RenderGroups[Cast_Uint(RENDER_GROUP::BLEND)].begin(), m_RenderGroups[Cast_Uint(RENDER_GROUP::BLEND)].end(), [=](const CGameObject* A, const CGameObject* B) {
			/* 거리기준이 필요하다 */
			CTransform* pTransform_A = A->GetTransform();
			CTransform* pTransform_B = B->GetTransform();
			CTransform* pTransform_Cam = RenderCam->GetTransform();

			float Length_A = pTransform_A->Get_Length(pTransform_Cam->Get_State_ToCal(STATE::POSITION));
			float Length_B = pTransform_B->Get_Length(pTransform_Cam->Get_State_ToCal(STATE::POSITION));
			/* 길이가 더 크다면 제일 앞쪽에 그려야한다 */
			return Length_A < Length_B;
			});


		Float4 CamAreaXY = RenderCam->Get_CameraRenderAreaXY_MinMax();

		for (auto& RenderObject : m_RenderGroups[Cast_Uint(RENDER_GROUP::BLEND)])
		{
			if (!RenderObject || !RenderObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))
				continue;
			/* 컬링이 On이 아니거나 On이지만 Area 안에들어와 있지 않는 애들은 걸러진다 */
			if (RenderObject->Get_IsRenderCulling() && !RenderObject->Get_IsInCamArea(CamAreaXY))
				continue;
			if (FAILED(RenderObject->Render(RenderCam)))
				return E_FAIL;
		}
	}

	return S_OK;
}

void CRenderManager::Set_GPU_BeforeLight()
{
}

#pragma region Measurement scaffolding

void CRenderManager::Set_PerCameraDeferred(bool isPerCamera)
{
	if (m_isPerCameraDeferred == isPerCamera) return;
	Flush_GpuAverage();
	m_isPerCameraDeferred = isPerCamera;
}

void CRenderManager::Set_RenderMode(RENDER_MODE eMode)
{
	if (m_eRenderMode == eMode) return;

	Flush_GpuAverage();
	m_eRenderMode = eMode;
	m_isPerCameraDeferred = (eMode == RENDER_MODE::PERCAM);
}

/* ---- A-plan --------------------------------------------------------------
   Keeps the shared G-Buffer. Clears only each camera's rect, records the camera
   index into Depth.z, and the light pass discards pixels owned by someone else.
   Combine stays a single fullscreen draw.                                   */
HRESULT CRenderManager::RenderGruop_APlan(const vector<CCameraObjectBase*>& vecRenderCam)
{
	if (vecRenderCam.empty()) return S_OK;

	if (FAILED(Render_Priority(vecRenderCam)))
		return E_FAIL;

	/* --- geometry : shared target, per-camera rect clear --- */
	if (FAILED(m_pGameManager->RenderTargetManager_Begin_MRT_NoClear(L"MRT_GameObject")))
		return E_FAIL;

	for (auto& RenderCam : vecRenderCam)
	{
		if (!RenderCam) continue;

		CONST VIEWPORT_DESC& tVP = RenderCam->Get_CamViewPort();
		RECT_DESC tRect{ (Diligent::Int32)tVP.TopLeftX, (Diligent::Int32)tVP.TopLeftY,
						  (Diligent::Int32)(tVP.TopLeftX + tVP.Width), (Diligent::Int32)(tVP.TopLeftY + tVP.Height) };

		m_pGameManager->RenderTargetManager_Clear_MRT_Rect(L"MRT_GameObject", tRect);

		/* later cameras must win the overlap, so depth restarts for each camera */
		DepthStencilView* pDSV = nullptr;
		CGraphicDevice::Get_Instance()->OM_GetRenderTargets(0, nullptr, &pDSV);
		if (pDSV)
		{
			m_pDeviceContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			pDSV->Release();
		}

		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();

		Float4 CamAreaXY = RenderCam->Get_CameraRenderAreaXY_MinMax();

		for (auto& RenderObject : m_RenderGroups[Cast_Uint(RENDER_GROUP::NONBLEND)])
		{
			if (!RenderObject || !RenderObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))		continue;
			if (RenderObject->Get_IsRenderCulling() && !RenderObject->Get_IsInCamArea(CamAreaXY))	continue;
			if (FAILED(RenderObject->Render(RenderCam)))											return E_FAIL;
		}
	}

	if (FAILED(m_pGameManager->RenderTargetManager_End_MRT()))
		return E_FAIL;

	/* --- lighting : one accumulation target, per-camera draws with index reject --- */
	if (FAILED(m_pGameManager->RenderTargetManager_Begin_MRT(L"MRT_LightAcc")))
		return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Normal", "g_NormalTexture", m_pShader)))
		return E_FAIL;
	if (FAILED(m_pGameManager->RenderTargetManager_Bind_ShaderResource(L"Target_Depth", "g_DepthTexture", m_pShader)))
		return E_FAIL;

	m_pGameManager->GraphicDevice_Set_GPU_OriginViewPort();
	m_pShader->Set_GPU_Transform(m_matWorld);
	m_pShader->Set_GPU_Canvas(m_matProj);
	m_pShader->Set_GPU_Matrix("g_GlobalInvProjMatrix", m_matGlobalInvProj);

	for (auto& RenderCam : vecRenderCam)
	{
		if (!RenderCam) continue;

		RenderCam->Set_GPU_Camera();
		RenderCam->Set_ScissorRects();
		m_pGameManager->LightManager_Render_Light(m_pShader, m_pBuffer, RenderCam);
	}

	CGraphicDevice::Get_Instance()->RS_SetScissorRects(0, nullptr);

	if (FAILED(m_pGameManager->RenderTargetManager_End_MRT()))
		return E_FAIL;

	/* --- combine : one fullscreen draw --- */
	if (FAILED(Render_Combind(nullptr)))
		return E_FAIL;

	if (FAILED(Render_NoneLight(vecRenderCam)))
		return E_FAIL;
	if (FAILED(Render_Blend(vecRenderCam)))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(Render_Debug(vecRenderCam)))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderManager::Ready_GpuTimer()
{
	/* 원본 : D3D11 TIMESTAMP_DISJOINT + TIMESTAMP 쿼리 → Diligent DURATION 쿼리 (지원하지 않는 장치는 측정 생략) */
	if (m_pDevice->GetDeviceInfo().Features.DurationQueries)
	{
		Diligent::QueryDesc tDesc{};
		tDesc.Name = "GPU Frame Timer";
		tDesc.Type = Diligent::QUERY_TYPE_DURATION;

		for (auto& Frame : m_tGpuTimer)
			m_pDevice->CreateQuery(tDesc, Frame.pDuration.ReleaseAndGetAddressOf());
	}

	m_ofsGpuLog.open("GpuFrameTime.csv", std::ios::out | std::ios::trunc);
	if (m_ofsGpuLog.is_open())
		m_ofsGpuLog << "Sample,Mode,CameraCount,GpuMS" << '\n';

	return S_OK;
}

void CRenderManager::GpuTimer_Begin(uint32 iCamCount)
{
	GPU_TIMER_FRAME& tFrame = m_tGpuTimer[m_iGpuTimerIndex];
	tFrame.iCamCount = iCamCount;
	tFrame.iMode     = static_cast<int>(m_eRenderMode);

	if (tFrame.pDuration != nullptr)
		m_pDeviceContext->BeginQuery(tFrame.pDuration.Get());
}

void CRenderManager::GpuTimer_End()
{
	GPU_TIMER_FRAME& tFrame = m_tGpuTimer[m_iGpuTimerIndex];
	if (tFrame.pDuration == nullptr)
		return;

	m_pDeviceContext->EndQuery(tFrame.pDuration.Get());
	tFrame.isInFlight = true;

	m_iGpuTimerIndex = (m_iGpuTimerIndex + 1) % GPUTIMER_DEPTH;

	GPU_TIMER_FRAME& tRead = m_tGpuTimer[m_iGpuTimerIndex];
	if (!tRead.isInFlight || tRead.pDuration == nullptr) return;

	Diligent::QueryDataDuration tData{};
	if (!tRead.pDuration->GetData(&tData, sizeof(tData), false))
		return;

	tRead.isInFlight = false;
	if (tData.Frequency == 0) return;

	const double dMilliSec = double(tData.Duration) / double(tData.Frequency) * 1000.0;

	m_dGpuTimeSum += dMilliSec;
	++m_iGpuSampleCount;

	if (m_ofsGpuLog.is_open())
		m_ofsGpuLog << m_iGpuSampleCount << ','
					<< (tRead.iMode == 2 ? "PERCAM" : tRead.iMode == 1 ? "APLAN" : "SHARED") << ','
					<< tRead.iCamCount << ','
					<< dMilliSec << '\n';
}

void CRenderManager::Flush_GpuAverage()
{
	if (!m_ofsGpuLog.is_open() || m_iGpuSampleCount == 0) return;

	m_ofsGpuLog << "AVG," << m_strBenchLabel << ','
				<< (m_eRenderMode == RENDER_MODE::PERCAM ? "PERCAM" : m_eRenderMode == RENDER_MODE::APLAN ? "APLAN" : "SHARED") << ','
				<< m_iGpuSampleCount << ','
				<< (m_dGpuTimeSum / m_iGpuSampleCount) << '\n';
	m_ofsGpuLog.flush();

	m_iGpuSampleCount = 0;
	m_dGpuTimeSum     = 0.0;
}

#pragma endregion

CRenderManager* CRenderManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CRenderManager* pRenderManager = new CRenderManager(pDevice, pContext);

	if (FAILED(pRenderManager->Initialize()))
	{
		Safe_Release(pRenderManager);

		MSG_BOX("Render Manager is failed to Create");

		return nullptr;
	}

	return pRenderManager;
}

void CRenderManager::Free()
{
	__super::Free();

	Flush_GpuAverage();
	if (m_ofsGpuLog.is_open()) m_ofsGpuLog.close();

	Safe_Release(m_pGameManager);


	Safe_Release(m_pBuffer);
	Safe_Release(m_pShader);
}


#ifdef _DEBUG

HRESULT CRenderManager::Add_DebugComponent(CComponent* pComponent)
{
	NULLPTR_RETURN_EFAIL(pComponent);

	m_vecDebugComponent.push_back(pComponent);
	Safe_AddRef(pComponent);

	return S_OK;
}

HRESULT CRenderManager::Render_Debug(const vector<CCameraObjectBase*>& vecRenderCam)
{
	NULLPTR_RETURN_EFAIL(m_pShader); NULLPTR_RETURN_EFAIL(m_pBuffer);

	/* DebugRender */

	//if (FAILED(Render_DebugComponent(vecRenderCam)))
	//	return E_FAIL;

	//if (FAILED(m_pBuffer->Bind_Resource()))
	//	return E_FAIL;
	//if (FAILED(m_pGameManager->GraphicDevice_Set_GPU_OriginViewPort()))
	//	return E_FAIL;
	//if (FAILED(m_pShader->Set_GPU_Canvas(m_matProj)))
	//	return E_FAIL;
	////if (FAILED(m_pGameManager->RenderTargetManager_Render_Debug(L"MRT_GameObject", m_pShader, m_pBuffer)))
	////	return E_FAIL;
	////if (FAILED(m_pGameManager->RenderTargetManager_Render_Debug(L"MRT_LightAcc" , m_pShader, m_pBuffer)))
	////	return E_FAIL;


	return S_OK;
}

HRESULT CRenderManager::Render_DebugComponent(const vector<CCameraObjectBase*>& vecRenderCam)
{
	/*  */
	for (auto& RenderCam : vecRenderCam)
	{
		RenderCam->Set_GPU_ViewPort();
		RenderCam->Set_GPU_Camera();

		for (auto& Com : m_vecDebugComponent)
		{
			if (Com)
				Com->Render(RenderCam);
		}
	}

	for(auto& Com : m_vecDebugComponent)
		Safe_Release(Com);
	m_vecDebugComponent.clear();

	return S_OK;
}
#endif

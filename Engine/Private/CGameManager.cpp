#include <random>
#include "CGameManager.h"
#include "CTimeManger.h"
#include "CGraphicDevice.h"
#include "CSceneManager.h"
#include "CPrototypeManager.h"
#include "CGameObjectManager.h"
#include "CPlayerManager.h"
#include "CRenderTargetManager.h"
#include "CRenderManager.h"
#include "CCameraManager.h"
#include "CInputManager.h"
#include "CScene.h"
#include "CRenderStateManager.h"
#include "CScreenShotManager.h"
#include "CResourceManager.h"
#include "CCollisionManager.h"
#include "CCellManager.h"
#include "CPipeLine.h"
#include "CUIManager.h"
#include "CFont.h"
#include "CFontManager.h"
#include "CComponent.h"
#include "CTexture.h"
#include "CShader.h"
#include "DirectXTK_Compat.h"

IMPLEMENT_SINGLETON(CGameManager)

CGameManager::CGameManager()
	: m_pGraphicDevice(nullptr) , m_pTimerManager(nullptr) , m_pSceneManager(nullptr) , m_pGameObjectManager(nullptr) , m_pPrototypeManager(nullptr)
	,  m_pInputManager(nullptr) , m_pRenderManger(nullptr) , m_pCameraManager(nullptr) , m_pRenderStateManager(nullptr)
	, m_pScreenShotManager(nullptr) , m_pPipeLine(nullptr) , m_iTotalSceneNum{}, m_iTotalRenderGroupNum{}, m_iDefaultWinSizeX{}, m_iDefaultWinSizeY{}
	, m_pLightManager(nullptr),m_isDebugMode{false} , m_pResourceManager(nullptr) , m_pCellManager(nullptr) , m_pPlayerManager(nullptr), m_pCollisionManager(nullptr)
	, m_pUIManager(nullptr), m_pFontManager(nullptr) , m_pRenderTargetManager(nullptr)
{
}


HRESULT CGameManager::Initialize_Engine(const Engine_Desc& tEngineDesc, ComPtr<Device>* ppDevice, ComPtr<DeviceContext>* ppDeviceContext)
{
	/* 멤버 변수 미리 태입 */
	m_iDefaultWinSizeX = tEngineDesc.tGraphicDevice_Desc.iWinSizeX;
	m_iDefaultWinSizeY = tEngineDesc.tGraphicDevice_Desc.iWinSizeY;
	m_iTotalSceneNum = tEngineDesc.tGameObjectManager_Desc.iTotalSceneNum;

	//Time Manager
	m_pTimerManager = CTimeManger::Create();
	NULLPTR_RETURN_EFAIL(m_pTimerManager);

	//Graphic Device
	m_pGraphicDevice = CGraphicDevice::Create(tEngineDesc.tGraphicDevice_Desc,ppDevice, ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pGraphicDevice);

	/* PipeLine */
	m_pPipeLine = CPipeLine::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pPipeLine);

	/* Resource Manager */
	m_pResourceManager = CResourceManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pResourceManager);

	/* RenderTarget Manager */
	m_pRenderTargetManager = CRenderTargetManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pRenderTargetManager);

	/* Scene Manager */
	m_pSceneManager = CSceneManager::Create();
	NULLPTR_RETURN_EFAIL(m_pSceneManager);

	/* Prototype Manager */
	m_pPrototypeManager = CPrototypeManager::Create(tEngineDesc.tPrototypeManager_Desc ,*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pPrototypeManager);

	/* UI Manager */
	m_pUIManager = CUIManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pPrototypeManager);

	/* Cell Manager */
	m_pCellManager = CCellManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pCellManager);

	/* CollisionManager */
	m_pCollisionManager = CCollisionManager::Create(tEngineDesc.tCollisionManager_Desc,*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pCollisionManager);

	/* GameObject Manager */
	m_pGameObjectManager = CGameObjectManager::Create(tEngineDesc.tGameObjectManager_Desc,*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pGameObjectManager);

	/* Player Manager */
	m_pPlayerManager = CPlayerManager::Create();
	NULLPTR_RETURN_EFAIL(m_pPlayerManager);

	/* RenderManager */
	m_pRenderManger = CRenderManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pRenderManger);


	/* CameraManager */
	m_pCameraManager = CCameraManager::Create(*ppDevice,*ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pCameraManager);


	/* Light Manager */
	m_pLightManager = CLightManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pLightManager);


	/* ScreenShotManager */
	m_pScreenShotManager = CScreenShotManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pScreenShotManager);


	/* RenderStateManager */
	m_pRenderStateManager = CRenderStateManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pRenderStateManager);


	/* Input Manager */
	m_pInputManager = CInputManager::Create(tEngineDesc.tGraphicDevice_Desc.hInst,tEngineDesc.tGraphicDevice_Desc.hWnd, *ppDevice, *ppDeviceContext,tEngineDesc.tGraphicDevice_Desc.iWinSizeX,tEngineDesc.tGraphicDevice_Desc.iWinSizeY);
	NULLPTR_RETURN_EFAIL(m_pInputManager);


	/* Font Manager */
	m_pFontManager = CFontManager::Create(*ppDevice, *ppDeviceContext);
	NULLPTR_RETURN_EFAIL(m_pFontManager);

	return S_OK;
}

#pragma region  Engine_Function
void CGameManager::Pre_Update_Engine(float DT)
{
	/* Key State Update */
	m_pInputManager->Update(DT);
	/* ---------------- */

	m_pSceneManager->Pre_Update(DT);

	m_pCameraManager->Pre_Update(DT);

	m_pUIManager->Pre_Update(DT);
}

void CGameManager::Update_Engine(float DT)
{
	/* Scene 내부에서 GameObjectManager 업데이트를 돌려준다. */
	m_pSceneManager->Update(DT);

	m_pCameraManager->Update(DT);

	m_pUIManager->Update(DT);
}

void CGameManager::Late_Update_Engine(float DT)
{
	m_pCollisionManager->Collision();

	m_pSceneManager->Late_Update(DT);

	m_pCameraManager->Late_Update(DT);

	m_pPipeLine->Update(DT);

	m_pUIManager->Late_Update(DT);
}

void CGameManager::Add_RenderGroup()
{
	m_pSceneManager->Add_RenderGroup();
}

HRESULT CGameManager::Draw()
{
	/* Render 시작전 PipeLine Setting */
	//m_pPipeLine->Bind_PipeLine();

	/* 카메라 매니저 렌더 호출 */
	m_pCameraManager->Render();

	/* 카메라가 바꿔놓은 뷰포트를 초기화시켜준다 */
	m_pGraphicDevice->Set_GPU_OriginViewPort();

	/* 그다음 UI 렌더 */
	m_pUIManager->Render();

	/* 각 Scene 고유의 Render */
	m_pSceneManager->Render();

	return S_OK;
}

int32 CGameManager::Get_RandomInt(int32 iMin, int32 iMax)
{
	return rand() % (iMax - iMin + 1 ) + iMin ;
}

float CGameManager::Get_Randomfloat(float fMin, float fMax)
{
	/* Random 으로뽑은 숫자를 정규화시킨다 0~1 사이값으로 그 이후 (최댓값 - 최소값) * 정규화값 + fMin을해준다.   */
	float fRandNormal = static_cast<float>(rand()) / RAND_MAX;
	return (fMax - fMin) * fRandNormal + fMin;
}

Float4 CGameManager::Get_RandomPosArea(const fVector4_1& vCenter, float minRange, float maxRange)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_real_distribution<float> angleDist(0.f,XM_2PI);
	float angle = angleDist(gen);

	std::uniform_real_distribution<float> radiusDist(minRange,maxRange);
	float r = radiusDist(gen);

	float fTargetX = XMVectorGetX(vCenter) + ( r * cosf(angle));
	float fTargetZ = XMVectorGetZ(vCenter) + (r * sinf(angle));
	
	return Float4( fTargetX , XMVectorGetY(vCenter) , fTargetZ , 1.f );
} 

Float3 CGameManager::Get_RandomPosition(Float3 fRange, Float3 vCenter)
{
	float RandomX = this->Get_Randomfloat(vCenter.x -fRange.x * 0.5f, vCenter.x + fRange.x * 0.5f);
	float RandomY = this->Get_Randomfloat(vCenter.y -fRange.y * 0.5f, vCenter.y + fRange.y * 0.5f);
	float RandomZ = this->Get_Randomfloat(vCenter.z -fRange.z * 0.5f, vCenter.z + fRange.z * 0.5f);
	

	return Float3(RandomX,RandomY,RandomZ);
}

ShaderResourceView* CGameManager::Make_ShaderResourceViewColor(uint32 A, uint32 R, uint32 G, uint32 B, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	/* 원본과 동일하게 128 x 128 흰색 텍스처를 만든다 (원본도 ARGB 인자와 상관없이 흰색으로 채웠다) */
	const uint32 iWidth = 128, iHeight = 128;
	std::vector<uint32> vecPixel(iWidth * iHeight, 0xFFFFFFFF);

	Diligent::ITexture* pTexture2D = nullptr;
	if (FAILED(Create_TextureRGBA8(pDevice.Get(), iWidth, iHeight, vecPixel.data(), "ShaderResourceViewColor", &pTexture2D)))
		return nullptr;

	ShaderResourceView* pSRV = pTexture2D->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	if (pSRV != nullptr)
		pSRV->AddRef();
	Safe_Release(pTexture2D);

	return pSRV;
}


HRESULT CGameManager::EndFrame()
{
	/* 프레임 끝나기전 씬 전환 */
	if (FAILED(m_pSceneManager->ExecutePendingSceneChanges()))
		return E_FAIL;
	
	if (FAILED(m_pScreenShotManager->ExecutePending_ScreenShot()))
		return E_FAIL;

	if (FAILED(m_pRenderManger->Clean_RenderGroup()))
		return E_FAIL;

	/* Present */
	if (FAILED(m_pGraphicDevice->Present()))
		return E_FAIL;

	/* Collider 그룹 순회돌린뒤 DeadObject를 삭제시켜준다 */
	m_pCollisionManager->Reset_IsCollision();
	m_pCollisionManager->Clean_DeadCollision();
	m_pGameObjectManager->Clean_DeadObject();

	return S_OK;
}


#pragma endregion

#pragma region TimerManager

HRESULT CGameManager::TimerManager_Add_Timer(const wstring strKey)
{
	NULLPTR_RETURN_EFAIL(m_pTimerManager);

	return m_pTimerManager->Add_Timer(strKey);
}

float CGameManager::TimerManager_Get_TimeDelta(const wstring strKey)
{
	NULLPTR_RETURN_FLOAT(m_pTimerManager);
	

	return m_pTimerManager->Get_TimeDelta(strKey);
}

void CGameManager::TimerManager_Compute_TimeDelta(const wstring strKey)
{
	if (m_pTimerManager == nullptr)
		return;

	return m_pTimerManager->Compute_TimeDelta(strKey);
}

uint32 CGameManager::TimerManger_Get_FPS(const wstring strKey)
{
	return m_pTimerManager->Get_FPS(strKey);
}


#pragma endregion

#pragma region GraphicDevice

void   CGameManager::GraphicDevice_RenderBegin(const Color& ClearColor)
{
	m_pGraphicDevice->Clear_BackBuffer_View(&ClearColor);
	m_pGraphicDevice->Clear_DepthStencil_View();
}
void    CGameManager::GraphicDevice_RenderEnd()
{
	m_pGraphicDevice->Present();
}

HRESULT CGameManager::GraphicDevice_Get_BackBuffer(ComPtr<Texture2D>* pTexture2D, uint32 iFlag)
{
	NULLPTR_RETURN_EFAIL(pTexture2D);
	*pTexture2D = m_pGraphicDevice->Get_BackBuffer();
	return *pTexture2D != nullptr ? S_OK : E_FAIL;
}

void CGameManager::GraphicDevice_Resize(uint32 iWinSizeX, uint32 iWinSizeY)
{
	if (m_pGraphicDevice != nullptr)
		m_pGraphicDevice->Resize(iWinSizeX, iWinSizeY);
}

HWND CGameManager::GraphicDevice_Get_HWND()
{
	return m_pGraphicDevice->Get_HWND();
}

HANDLE CGameManager::GraphicDevice_Get_Handle()
{
	return m_pGraphicDevice->Get_Handle();
}
ComPtr<Device> CGameManager::GraphicDevice_Get_Device()
{
	return m_pGraphicDevice->Get_Device();
}
ComPtr<DeviceContext> CGameManager::GraphicDevice_Get_DeviceContext()
{

	return m_pGraphicDevice->Get_DeviceContext();
}
ComPtr<RTV> CGameManager::GraphicDevice_Get_RTV()
{
	return m_pGraphicDevice->GetRTV();
}
ComPtr<DepthStencilView> CGameManager::GraphicDevice_Get_DSV()
{
	return m_pGraphicDevice->GetDSV();
}
HRESULT CGameManager::GraphicDevice_Set_GPU_OriginViewPort()
{
	return m_pGraphicDevice->Set_GPU_OriginViewPort();
}
const VIEWPORT_DESC& CGameManager::GraphicDevice_Get_OriginViewPort()
{
	return m_pGraphicDevice->Get_OriginViewPort();
}
void CGameManager::CGraphicDevice_Set_ScissorRectsOrigin()
{
	return m_pGraphicDevice->Set_ScissorRectsOrigin();
}
#pragma endregion

#pragma region ResourceManager

CResource* CGameManager::ResourceManager_Get_Resource(const wstring& wstrKey, RESOURCE_TYPE eResourceType, bool isCopy, void* pArg)
{
	return m_pResourceManager->Get_Resource(wstrKey, eResourceType, isCopy, pArg);
}

HRESULT CGameManager::ResourceManager_Add_Resource(const wstring& wstrKey, CResource* pResource)
{
	return m_pResourceManager->Add_Resource(wstrKey, pResource);
}

#pragma endregion 

#pragma region SceneManager


HRESULT CGameManager::SceneManager_Register_SceneFactory(uint32 iSceneID, const SceneFactory& SceneCreateFuntion)
{
	return m_pSceneManager->Register_SceneFactory(iSceneID,SceneCreateFuntion);
}

void CGameManager::SceneManager_RequestChangeScene(const CreateScene_Desc& tDesc)
{
	m_pSceneManager->RequestChangeScene(tDesc);
}

HRESULT CGameManager::SceneManager_ExecutePendingSceneChanges()
{
	return m_pSceneManager->ExecutePendingSceneChanges();
}

#pragma endregion


#pragma region Collision Manager

HRESULT CGameManager::CollisionManager_Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID)
{
	return m_pCollisionManager->Register_CollisionTable(iSrcGroupID, iDstGroupID);
}

HRESULT CGameManager::CollisionManager_Add_Collider(uint32 iAddGroupID, CCollider* pCollider)
{
	return m_pCollisionManager->Add_Collider(iAddGroupID, pCollider);
}

void CGameManager::CollisionManager_RayCollision(uint32 iColliderGruopID, const RAY_INFO& tRayInfo, RayHit* pRayHit)
{
	return m_pCollisionManager->RayCollision(iColliderGruopID, tRayInfo, pRayHit);
}

void CGameManager::CollisionManager_Clear_Collision()
{
	return m_pCollisionManager->Clear_Collision();
}

#pragma endregion

#pragma region PrototypeManager
CBase* CGameManager::PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE eCloneProtoType, uint32 iCloneProtoSceneID, const wstring& strCloneProtoKey, void* pArg)
{
	return m_pPrototypeManager->Clone_Prototype(eCloneProtoType, iCloneProtoSceneID,strCloneProtoKey,pArg);
}

CBase* CGameManager::PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE eCloneProtoType, const wstring& strCloneProtoKey, void* pArg)
{
	return m_pPrototypeManager->Clone_Prototype_InGlobal(eCloneProtoType,strCloneProtoKey,pArg);
}

HRESULT CGameManager::PrototypeManager_Add_Prototype(PROTOTYPE_TYPE eAddProtoType, uint32 iAddProtoSceneID, const wstring& strAddProtoKey, CBase* pPrototype)
{
	return m_pPrototypeManager->Add_Prototype(eAddProtoType,iAddProtoSceneID,strAddProtoKey,pPrototype);
}
HRESULT CGameManager::PrototypeManager_Add_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType, const wstring& strAddProtoKey, CBase* pPrototype)
{
	return m_pPrototypeManager->Add_Prototype_InGlobal(eProtoType,strAddProtoKey,pPrototype);
}

void CGameManager::Clear_Prototype_CurSceneID(uint32 iCurSceneID)
{
	m_pPrototypeManager->Clear_Prototype_CurSceneID(iCurSceneID);
}


#ifdef _DEBUG

/* PrototypeManager  */
map<const wstring, CBase*>& CGameManager::PrototypeManager_GetGameObjectPrototypeList_ByScene(uint32 iSceneID)
{
	return m_pPrototypeManager->GetGameObjectPrototypeList_ByScene(iSceneID);
}
map<const wstring, CBase*>& CGameManager::PrototypeManager_GetComponentPrototypeList_ByScene(uint32 iSceneID)
{
	return m_pPrototypeManager->GetComponentPrototypeList_ByScene(iSceneID);
}
const map<const wstring, CBase*>& CGameManager::PrototypeManager_GetGlobalGameObjectPrototypeList()
{
	return m_pPrototypeManager->GetGlobalGameObjectPrototypeList();
}

const map<const wstring, CBase*>& CGameManager::PrototypeManager_GetGlobalComponentPrototypeList()
{
	return m_pPrototypeManager->GetGlobalComponentPrototypeList();
}

#endif // _DEBUG

#pragma endregion


#pragma region Render Manager

HRESULT CGameManager::RenderManager_RenderGruop(const vector<CCameraObjectBase*>& vecRenderCam)
{
	return m_pRenderManger->RenderGruop(vecRenderCam);
}

HRESULT CGameManager::RenderManager_RenderGruop(CCameraObjectBase* pRenderCam)
{
	return m_pRenderManger->RenderGruop(pRenderCam);
}

HRESULT CGameManager::RenderManager_RenderGruop_APlan(const vector<CCameraObjectBase*>& vecRenderCam)
{
	return m_pRenderManger->RenderGruop_APlan(vecRenderCam);
}

void CGameManager::RenderManager_Set_BenchLabel(const string& strLabel)
{
	m_pRenderManger->Set_BenchLabel(strLabel);
}

void CGameManager::RenderManager_Set_RenderMode(int32 iMode)
{
	m_pRenderManger->Set_RenderMode(static_cast<CRenderManager::RENDER_MODE>(iMode));
}

int32 CGameManager::RenderManager_Get_RenderMode()
{
	return static_cast<int32>(m_pRenderManger->Get_RenderMode());
}

void CGameManager::RenderManager_Set_PerCameraDeferred(bool isPerCamera)
{
	m_pRenderManger->Set_PerCameraDeferred(isPerCamera);
}

bool CGameManager::RenderManager_Get_PerCameraDeferred()
{
	return m_pRenderManger->Get_PerCameraDeferred();
}

void CGameManager::RenderManager_GpuTimer_Begin(uint32 iCamCount)
{
	m_pRenderManger->GpuTimer_Begin(iCamCount);
}

void CGameManager::RenderManager_GpuTimer_End()
{
	m_pRenderManger->GpuTimer_End();
}

HRESULT CGameManager::RenderManager_Add_RenderGroup(RENDER_GROUP eRenderGruop, CGameObject* pGameObject)
{
	return m_pRenderManger->Add_RenderGroup(eRenderGruop,pGameObject);
}

#ifdef _DEBUG

HRESULT CGameManager::RenderManager_Add_DebugComponent(CComponent* pComponent)
{
	return m_pRenderManger->Add_DebugComponent(pComponent);
}

HRESULT CGameManager::RenderManager_Render_DebugPass(const vector<CCameraObjectBase*>& vecRenderCam)
{
	return m_pRenderManger->Render_DebugPass(vecRenderCam);
}
#endif

HRESULT CGameManager::RenderManager_Clean_RenderGroup()
{
	return m_pRenderManger->Clean_RenderGroup();
}
#pragma endregion


#pragma region RenderTargetManager
HRESULT CGameManager::RenderTargetManager_Add_RenderTarget(const wstring& wstrAddKey, uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat)
{
	return m_pRenderTargetManager->Add_RenderTarget(wstrAddKey,iWidth,iHeight,vColor,eFormat);
}

HRESULT CGameManager::RenderTargetManager_Add_MRT(const wstring& wstrAddMRTKey, const wstring& wstrFindRenderTargetKey)
{
	return m_pRenderTargetManager->Add_MRT(wstrAddMRTKey,wstrFindRenderTargetKey);
}

HRESULT CGameManager::RenderTargetManager_Begin_MRT_NoClear(const wstring& wstrFindMRT)
{
	return m_pRenderTargetManager->Begin_MRT_NoClear(wstrFindMRT);
}

HRESULT CGameManager::RenderTargetManager_Clear_MRT_Rect(const wstring& wstrFindMRT, const RECT_DESC& tRect)
{
	return m_pRenderTargetManager->Clear_MRT_Rect(wstrFindMRT, tRect);
}

HRESULT CGameManager::RenderTargetManager_Begin_MRT(const wstring& wstrFindMRT)
{
	return m_pRenderTargetManager->Begin_MRT(wstrFindMRT);
}

HRESULT CGameManager::RenderTargetManager_End_MRT()
{
	return m_pRenderTargetManager->End_MRT();
}

HRESULT CGameManager::RenderTargetManager_Bind_ShaderResource(const wstring& wstrFindRenderTargetKey, const string& strBindTexName, CShader* pShader)
{
	return m_pRenderTargetManager->Bind_ShaderResource(wstrFindRenderTargetKey, strBindTexName, pShader);
}

ShaderResourceView* CGameManager::RenderTargetManager_Get_ShaderResourceView(const wstring& wstrFindRenderTargetKey)
{
	return m_pRenderTargetManager->Get_ShaderResourceView(wstrFindRenderTargetKey);
}


#ifdef _DEBUG
HRESULT CGameManager::RenderTargetManager_Ready_Debug(const wstring& wstrFindRenderTargetKey, float fX, float fY, float fSizeX, float fSizeY)
{
	return m_pRenderTargetManager->Ready_Debug(wstrFindRenderTargetKey,fX,fY,fSizeX,fSizeY);
}
HRESULT CGameManager::RenderTargetManager_Render_Debug(const wstring& wstrFindRenderTargtKey, CShader* pShader, CVIBuffer_Rect* pBuffer)
{
	return m_pRenderTargetManager->Render_Debug(wstrFindRenderTargtKey,pShader,pBuffer);
}
#endif

#pragma endregion


#pragma region Camera Manager


HRESULT CGameManager::CameraManager_MakeDefaultCamera(CONST CDefaultCameraObject::DefaultCameraObject_Dest* pDesc)
{
	return m_pCameraManager->MakeDefaultCamera(pDesc);
}

HRESULT CGameManager::CameraManager_Add_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode, CCameraObjectBase* pCamera)
{
	return m_pCameraManager->Add_Camera(wstrKey, eUpdateMode,pCamera);
}

HRESULT CGameManager::CameraManager_Remove_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode)
{
	return m_pCameraManager->Remove_Camera(wstrKey, eUpdateMode);
}

CCameraObjectBase* CGameManager::CameraManager_Get_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode)
{
	return m_pCameraManager->Get_Camera(wstrKey,eUpdateMode);
}

void CGameManager::CameraManager_Set_CameraRenderActive(const wstring& wstrKey, UPDATE_MODE eUpdateMode, bool isRednerActive)
{
	return m_pCameraManager->Set_CameraRenderActive(wstrKey, eUpdateMode,isRednerActive);
}

const array<unordered_map<wstring, CCameraObjectBase*>, Cast_Uint(UPDATE_MODE::END)>& CGameManager::CameraManager_Get_CameraList()
{
	return m_pCameraManager->Get_CameraList();
}

CCameraObjectBase* CGameManager::CameraManager_Get_CurRenderingCamera()
{
	return m_pCameraManager->Get_CurRenderingCamera();
}

HRESULT CGameManager::CameraManager_MakeDefaultUICamera(const CUICameraObject::UICameraObject_Desc* pUICameraDest)
{
	return m_pCameraManager->MakeDefaultUICamera(pUICameraDest);
}

HRESULT CGameManager::CameraManager_Render()
{
	return m_pCameraManager->Render();
}

#pragma endregion



#pragma region LightMangaer

const LIGHT_GROUP_BUF* CGameManager::LightManager_Get_LightGroupBuffer()
{
	return m_pLightManager->Get_LightGroupBuffer();
}

CLight* CGameManager::LightManager_Get_Light(LIGHT_TYPE eType, uint32 iIndex)
{
	return m_pLightManager->Get_Light(eType,iIndex);
}

const LIGHT_BUF_DATA& CGameManager::LightManager_Get_LightBufferData(LIGHT_TYPE eType, uint32 iIndex)
{
	return m_pLightManager->Get_LightBufferData(eType,iIndex);
}

HRESULT CGameManager::LightManager_Add_Light(const CLight::Light_Desc& tDesc)
{
	return m_pLightManager->Add_Light(tDesc);
}

HRESULT CGameManager::LightManager_Render_Light(CShader* pSader, CVIBuffer_Rect* pVIBuffer, CCameraObjectBase* pRenderCam)
{
	return m_pLightManager->Render_Light(pSader, pVIBuffer, pRenderCam);
}

list<CLight*> CGameManager::LightManager_Get_LightList(LIGHT_TYPE eType)
{
	return m_pLightManager->Get_LightList(eType);
}

HRESULT CGameManager::LightManager_Clear_Light()
{
	return m_pLightManager->Clear_Light();
}

HRESULT CGameManager::LightManager_Add_Light(CLight* pLight)
{
	return m_pLightManager->Add_Light(pLight);
}

#pragma endregion

#pragma	region PipeLine

HRESULT CGameManager::PipeLine_Set_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, const XMatrix_1& TransformMatrix)
{
	return m_pPipeLine->Set_PipeLineMatrix(wstrFindKey,eTransformMatrix,TransformMatrix);
}

HRESULT CGameManager::PipeLine_Set_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, const Float4x4& TransformMatrix)
{
	return m_pPipeLine->Set_PipeLineMatrix(wstrFindKey, eTransformMatrix,TransformMatrix);
}

Float4x4 CGameManager::PipeLine_Get_PipeLineMatrix(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) CONST
{
	return m_pPipeLine->Get_PipeLineMatrix(wstrFindKey,eTransformMatrix, isInverse);
}

const Float4x4* CGameManager::PipeLine_Get_PipeLineMatrixPtr(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) const
{
	return m_pPipeLine->Get_PipeLineMatrixPtr(wstrFindKey,eTransformMatrix,isInverse);
}

Float4 CGameManager::PipeLine_Get_CameraPosition(const std::wstring& wstrFindKey) CONST
{
	return m_pPipeLine->Get_CameraPosition(wstrFindKey);
}

HRESULT CGameManager::PipeLine_Link_EffectConstantBuffer(GLOBAL_CB_TYPE eCBType, CShader* pShader)
{
	return m_pPipeLine->Link_EffectConstantBuffer(eCBType,pShader);
}

HRESULT CGameManager::PipeLine_Bind_PipeLine()
{
	return m_pPipeLine->Bind_PipeLine();
}



HRESULT CGameManager::PipeLine_Set_GPU_CameraBuffer(const wstring& wstrFindKey, const CAM_SHADER_INFO& tCamInfo)
{
	return m_pPipeLine->Set_GPU_CameraBuffer(wstrFindKey,tCamInfo);
}

HRESULT CGameManager::PipeLine_Set_GPU_LightGroupBuffer(const LIGHT_GROUP_BUF& tLightGroupBuf)
{
	return m_pPipeLine->Set_GPU_LightGroupBuffer(tLightGroupBuf);
}

HRESULT CGameManager::PipeLine_Set_GPU_MouseBuffer(const MOUSE_BUF_DATA& tData)
{
	return m_pPipeLine->Set_GPU_MouseBuffer(tData);
}

const void CGameManager::PipeLine_Clear_PipeLine()
{
	return m_pPipeLine->Clear_PipeLine();
}


#pragma endregion

#pragma region ScreenShot Manager


HRESULT CGameManager::ScreenShotManager_Add_ScreenShot(const wstring& strKey, CScreenShot* pTexture)
{
	return m_pScreenShotManager->Add_ScreenShot(strKey, pTexture);
}

CScreenShot* CGameManager::ScreenShotManager_Get_ScreenShot(const wstring& strKey)
{
	return m_pScreenShotManager->Get_ScreenShot(strKey);
}


void CGameManager::ScreenShotManager_Request_ScreenShot(bool isOverLay, CScreenShot** ppScreenShot)
{
	return m_pScreenShotManager->Request_ScreenShot(isOverLay, ppScreenShot);
}

HRESULT CGameManager::ScreenShotManager_ScreenShot(bool isOverLay, CScreenShot** ppScreenShot)
{
	return m_pScreenShotManager->ScreenShot(isOverLay,ppScreenShot);
}

CScreenShot* CGameManager::ScreenShotMangaer_Get_OverLayScreenShot()
{
	return m_pScreenShotManager->Get_OverLayScreenShot();
}

#pragma endregion

#pragma region RenderStateManager


HRESULT CGameManager::RenderStateManager_Set_GPU_RenderState(uint32 iIndex, STATE_TYPE eType)
{
	return m_pRenderStateManager->Set_GPU_RenderState(iIndex,eType);
}

HRESULT CGameManager::RenderStateManager_Add_RenderState(uint32 iIndex, FD CRenderState* pAddRenderState)
{
	return m_pRenderStateManager->Add_RenderState(iIndex, pAddRenderState);
}

FD CRenderState* CGameManager::RenderStateManager_Get_RenderState(uint32 iIndex, STATE_TYPE eType)
{
	return m_pRenderStateManager->Get_RenderState(iIndex,eType);
}


#pragma endregion

#pragma region Input Manager

bool CGameManager::InputManager_GetButtonPress(KEY_TYPE key)
{
	return m_pInputManager->GetButtonPress(key);
}
bool CGameManager::InputManager_GetButtonDown(KEY_TYPE key)
{
	return m_pInputManager->GetButtonDown(key);
}
bool CGameManager::InputManager_GetButtonUp(KEY_TYPE key)
{
	return m_pInputManager->GetButtonUp(key);
}

long CGameManager::InputManager_Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	return m_pInputManager->Get_DIMouseMove(eMouseState);
}

long CGameManager::InputManager_Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	return m_pInputManager->Get_DIMouseState(eMouse);
}

CONST POINT& CGameManager::InputManager_GetPreFrameMousePos()
{
	return m_pInputManager->Get_PreFrameMousePos();
}

CONST POINT& CGameManager::InputManager_GetMousePos()
{
	return m_pInputManager->Get_MousePos();
}

void CGameManager::InputManager_Mouse_Fix()
{
	return m_pInputManager->Mouse_Fix();
}

void CGameManager::InputManager_Mouse_Fix(uint32 iStartX, uint32 iStartY, uint32 iWinSizeX, uint32 iWinSizeY)
{
	return m_pInputManager->Mouse_Fix(iStartX, iStartY, iWinSizeX, iWinSizeY);
}

void CGameManager::InputManager_Mouse_Set_Show(bool isShow)
{
	return m_pInputManager->Mouse_Set_Show(isShow);
}

RAY_INFO CGameManager::InputManager_Get_LayWorld(const wstring& wstrCamName, UPDATE_MODE eType, RAY_TYPE eRayType)
{
	return m_pInputManager->Get_LayWorld(wstrCamName,eType,eRayType);
}

void CGameManager::InputManager_Set_NativeTouch(uint32 iSlot, bool isPressed, float fScreenX, float fScreenY)
{
	if (m_pInputManager != nullptr)
		m_pInputManager->Set_NativeTouch(iSlot, isPressed, fScreenX, fScreenY);
}

void CGameManager::InputManager_Set_NativeKey(KEY_TYPE eKey, bool isPressed)
{
	if (m_pInputManager != nullptr)
		m_pInputManager->Set_NativeKey(eKey, isPressed);
}

void CGameManager::InputManager_Update(float DT)
{
	return m_pInputManager->Update(DT);
}
#pragma endregion


#pragma region PlayerManager

HRESULT CGameManager::PlayerManager_Add_Player(const wstring& AddKey, CGameObject* pPlayer)
{
	return m_pPlayerManager->Add_Player(AddKey,pPlayer);
}

CGameObject* CGameManager::PlayerManager_Get_Player(const wstring& FindKey)
{
	return m_pPlayerManager->Get_Player(FindKey);
}

CGameObject* CGameManager::PlayerManager_Compute_CheckRange(const fVector4_1& vMonsterPos, float fMonsterRange,CGameObject* pCurTargetPlayer)
{
	return m_pPlayerManager->Compute_CheckRange(vMonsterPos,fMonsterRange, pCurTargetPlayer);
}

void CGameManager::PlayerManager_ClearPlayer()
{
	return m_pPlayerManager->Clear_Player();
}

HRESULT CGameManager::FontMananger_Add_Font(float fOriginFontSize,const wstring& wstrFindKey,const wstring& wstrFontPath)
{
	return m_pFontManager->Add_Font(fOriginFontSize,wstrFindKey,wstrFontPath);
}

CFont* CGameManager::FontMananger_Get_Font(const wstring& wstrFindKey)
{
	return m_pFontManager->Get_Font(wstrFindKey);
}

HRESULT CGameManager::FontMananger_Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{
	return m_pFontManager->Draw(wstringFindKey,pText,vPosition,vColor,eType);
}

HRESULT CGameManager::FontMananger_Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const Float2& vSize, const fVector4_1& vColor, FONT_RENDER_TYPE eType)
{
	return m_pFontManager->Draw(wstringFindKey, pText, vPosition, vSize ,vColor, eType);
}

#pragma endregion



#pragma region GameObjectManager
CGameObject* CGameManager::GameObjectManager_Add_GameObject(uint32 iFindProtoSceneID, const wstring& strCloneProtokey, uint32 iAddCloneSceneID, const wstring& strAddLayerKey, void* pArg)
{
	return m_pGameObjectManager->Add_GameObject(iFindProtoSceneID, strCloneProtokey,iAddCloneSceneID,strAddLayerKey,pArg);
}
CGameObject* CGameManager::GameObjectManager_Add_GameObject_ByGlobalProto(const wstring& strFindProtoKey, const wstring& strAddLayerKey, void* pArg, uint32 iAddCloneSceneID)
{
	return m_pGameObjectManager->Add_GameObject_ByGlobalProto(strFindProtoKey,strAddLayerKey,pArg,iAddCloneSceneID);
}
void CGameManager::GameObjectManager_Pre_Update(uint32 iSceneID, float DT)
{
	return m_pGameObjectManager->Pre_Update(iSceneID, DT);
}
void CGameManager::GameObjectManager_Update(uint32 iSceneID, float DT)
{
	return m_pGameObjectManager->Update(iSceneID,DT);
}
void CGameManager::GameObjectManager_Late_Update(uint32 iSceneID, float DT)
{
	return m_pGameObjectManager->Late_Update(iSceneID,DT);
}
HRESULT CGameManager::GameObjectManager_Add_RenderGroup(uint32 iSceneID)
{
	return m_pGameObjectManager->Add_RenderGroup(iSceneID);
}
HRESULT CGameManager::GameObjectManager_Clear_GameObjectByScneeID(uint32 iSceneID)
{
	return m_pGameObjectManager->Clear_GameObjectByScneeID(iSceneID);
}
const CLayer* CGameManager::GameObjectManager_Get_Layer(uint32 iSceneID, const wstring& wstrLayerName)
{
	return m_pGameObjectManager->Get_Layer(iSceneID,wstrLayerName);
}
CGameObject* CGameManager::GameObjectManager_Find_GameObject_InGlobal(const wstring& strFindLayerKey, const wstring& strGameObjectName)
{
	return m_pGameObjectManager->Find_GameObject_InGlobal(strFindLayerKey,strGameObjectName);
}

CGameObject* CGameManager::GameObjectManager_Find_GameObject_InGlobal(const wstring& strFindLayerKey, CGameObject* pFindGameObject)
{
	return m_pGameObjectManager->Find_GameObject_InGlobal(strFindLayerKey, pFindGameObject);
}

CGameObject* CGameManager::GameObjectManager_Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, CGameObject* pFindGameObject)
{
	return m_pGameObjectManager->Find_GameObject(iFindSceneID, strFindLayerKey, pFindGameObject);
}

CGameObject* CGameManager::GameObjectManager_Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, const wstring& strGameObjectName)
{
	return m_pGameObjectManager->Find_GameObject(iFindSceneID, strFindLayerKey, strGameObjectName);
}

void CGameManager::GameObjectManager_Add_ColliderGroup(uint32 iSceneID)
{
	return m_pGameObjectManager->Add_ColliderGroup(iSceneID);
}

#ifdef _DEBUG
const map<const wstring, CLayer*>& CGameManager::GameObjectManager_GetCurSceneLayer(uint32 iCurSceneID)
{
	return m_pGameObjectManager->GetCurSceneLayer(iCurSceneID);
}
const vector<map<const wstring, CLayer*>>& CGameManager::GameObjectManager_Get_AllSceneLayer()
{
	return m_pGameObjectManager->Get_AllSceneLayer();
}
const map<const wstring, CLayer*>& CGameManager::GameObjectManager_GetGlobalLayer()
{
	return m_pGameObjectManager->GetGlobalLayer();
}


#endif // _DEBUG
#pragma endregion

#pragma region  UIManager

HRESULT CGameManager::UIManager_Add_Canvas(uint32 iIndex, CCanvas* pCanvas)
{
	return m_pUIManager->Add_Canvas(iIndex,pCanvas);
}
CCanvas* CGameManager::UIManager_Get_Canvas(uint32 iIndex)
{
	return m_pUIManager->Get_Canvas(iIndex);
}

void CGameManager::UIManager_Release_Canvas(uint32 iIndex)
{
	return m_pUIManager->Release_Canvas(iIndex);
}

void CGameManager::UIManager_Fade_Out(float fTime)
{
	return m_pUIManager->Fade_Out(fTime);
}

void CGameManager::UIManager_Fade_In(float fTime)
{
	return m_pUIManager->Fade_In(fTime);
}

#pragma endregion

#pragma region SceneManager

vector<CScene*>& CGameManager::SceneManager_GetStackScene()
{
	return m_pSceneManager->GetStackScene();
}

uint32 CGameManager::SceneManager_GetCurrentSceneID()
{
	return m_pSceneManager->GetCurrentSceneID();
}

CScene* CGameManager::SceneManager_GetCurScene()
{
	return m_pSceneManager->GetCurScene();
}

#pragma endregion

#pragma region CellManager

const vector<CCell*>& CGameManager::CellManager_Get_Cells(const wstring& wstrFindKey)
{
	return m_pCellManager->Get_Cells(wstrFindKey);
}

HRESULT CGameManager::CellManager_Add_Cells(const wstring& wstrCellPath, const wstring& wstrAddKey)
{
	return m_pCellManager->Add_Cells(wstrCellPath,wstrAddKey);
}

bool CGameManager::CellManager_Get_IsCellRender() const
{
	return m_pCellManager->Get_IsCellRender();
}

void CGameManager::CellManager_Set_IsCellRender(bool isRender)
{
	return m_pCellManager->Set_IsCellRender(isRender);
}
#pragma endregion

HRESULT CGameManager::Reaydy_Default(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	HRESULT hr{ E_FAIL };

	hr = m_pResourceManager->Ready_DefaultResource();
	
	hr = m_pPrototypeManager->Ready_DefaultPrototype(pDevice, pContext);

	hr = m_pRenderManger->Ready_Default();

	hr = m_pUIManager->Ready_DefaultUI();

	return hr;
}



void CGameManager::Release_Manager()
{
	Safe_Release(m_pTimerManager);
	Safe_Release(m_pSceneManager);
	Safe_Release(m_pUIManager);
	Safe_Release(m_pCollisionManager);
	Safe_Release(m_pPlayerManager);
	Safe_Release(m_pGameObjectManager);
	Safe_Release(m_pPrototypeManager);
	Safe_Release(m_pCellManager);
	Safe_Release(m_pRenderTargetManager);
	Safe_Release(m_pRenderManger);
	Safe_Release(m_pCameraManager);
	Safe_Release(m_pRenderStateManager);
	Safe_Release(m_pScreenShotManager);
	Safe_Release(m_pResourceManager);
	Safe_Release(m_pFontManager);
	Safe_Release(m_pLightManager);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pInputManager);
	CShader::Release_SharedResources();
	DirectX::PrimitiveBatch_ReleaseResources();
	Safe_Release(m_pGraphicDevice);


	CGameManager::GetInstance()->DestroyInstance();
}

void CGameManager::ChangeScene_Clear_CurSceneObject(uint32 iCurSceneID)
{
	m_pPrototypeManager->Clear_Prototype_CurSceneID(iCurSceneID);
	m_pGameObjectManager->Clear_GameObjectByScneeID(iCurSceneID);
}

void CGameManager::Free()
{
	__super::Free();
}

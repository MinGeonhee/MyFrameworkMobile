#include "CGameManager.h"

#include "CCameraManager.h"
#include "CCellManager.h"
#include "CCollisionManager.h"
#include "CComponent.h"
#include "CDebugOverlay.h"
#include "CFontManager.h"
#include "CGameObjectManager.h"
#include "CGraphicDevice.h"
#include "CInputManager.h"
#include "CLightManager.h"
#include "CPlayerManager.h"
#include "CPrototypeManager.h"
#include "CRenderManager.h"
#include "CRenderStateManager.h"
#include "CRenderTargetManager.h"
#include "CResourceManager.h"
#include "CResource.h"
#include "CTexture.h"
#include "CSceneManager.h"
#include "CScreenShotManager.h"
#include "CSoundManager.h"
#include "CTimeManger.h"
#include "CUIManager.h"
#include "CGameObject.h"
#include "CScene.h"

IMPLEMENT_SINGLETON(CGameManager)

CGameManager::CGameManager() = default;

HRESULT CGameManager::Initialize_Engine(const Engine::Engine_Desc& tEngineDesc)
{
    m_pGraphicDevice = new CGraphicDevice{};
    if (m_pGraphicDevice == nullptr || FAILED(m_pGraphicDevice->Initialize(tEngineDesc)))
    {
        Free();
        return E_FAIL;
    }

    if (FAILED(Initialize_Manager(tEngineDesc)))
    {
        Free();
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CGameManager::Initialize_Manager(const Engine::Engine_Desc& tEngineDesc)
{
    m_pTimerManager = CTimeManger::Create();
    m_pSceneManager = CSceneManager::Create();
    m_pGameObjectManager = CGameObjectManager::Create(
        tEngineDesc.tPrototypeManagerDesc.iTotalLevelCount,
        m_pGraphicDevice->Get_Device(),
        m_pGraphicDevice->Get_DeviceContext());
    m_pPrototypeManager = CPrototypeManager::Create(
        tEngineDesc.tPrototypeManagerDesc,
        m_pGraphicDevice->Get_Device(),
        m_pGraphicDevice->Get_DeviceContext());
    m_pInputManager = CInputManager::Create(tEngineDesc.pNativeWindow);
    m_pRenderManager = CRenderManager::Create();
    m_pRenderStateManager = CRenderStateManager::Create();
    m_pRenderTargetManager = CRenderTargetManager::Create(
        m_pGraphicDevice->Get_Device(),m_pGraphicDevice->Get_DeviceContext(),m_pGraphicDevice->Get_SwapChain());
    m_pCameraManager = CCameraManager::Create();
    m_pScreenShotManager = CScreenShotManager::Create();
    m_pResourceManager = CResourceManager::Create(
        m_pGraphicDevice->Get_Device(),
        m_pGraphicDevice->Get_DeviceContext());
    m_pLightManager = CLightManager::Create();
    m_pCellManager = CCellManager::Create();
    m_pPlayerManager = CPlayerManager::Create();
    m_pCollisionManager = CCollisionManager::Create();
    m_pUIManager = CUIManager::Create();
    m_pFontManager = CFontManager::Create();
    m_pSoundManager = CSoundManager::GetInstance();

    if (m_pTimerManager == nullptr || m_pSceneManager == nullptr ||
        m_pGameObjectManager == nullptr || m_pPrototypeManager == nullptr ||
        m_pInputManager == nullptr || m_pRenderManager == nullptr ||
        m_pRenderStateManager == nullptr || m_pRenderTargetManager == nullptr ||
        m_pCameraManager == nullptr || m_pScreenShotManager == nullptr ||
        m_pResourceManager == nullptr || m_pLightManager == nullptr ||
        m_pCellManager == nullptr || m_pPlayerManager == nullptr ||
        m_pCollisionManager == nullptr || m_pUIManager == nullptr ||
        m_pFontManager == nullptr || m_pSoundManager == nullptr)
        return E_FAIL;

    if (FAILED(m_pTimerManager->Add_Timer(L"Timer_Default")) ||
        FAILED(m_pPrototypeManager->Ready_DefaultPrototype()) ||
        FAILED(m_pResourceManager->Ready_DefaultResource()) ||
        FAILED(m_pRenderStateManager->Ready_Default()) ||
        FAILED(m_pRenderTargetManager->Ready_Default()) ||
        FAILED(m_pRenderManager->Ready_Default()) ||
        FAILED(m_pUIManager->Ready_DefaultUI()) ||
        FAILED(m_pFontManager->Ready_Default()) ||
        FAILED(m_pSoundManager->Initialize(32)))
        return E_FAIL;

    m_pDebugOverlay = CDebugOverlay::Create(
        m_pGraphicDevice->Get_Device(),
        m_pGraphicDevice->Get_DeviceContext());
    if (m_pDebugOverlay == nullptr)
        return E_FAIL;

    return S_OK;
}

void CGameManager::Pre_Update_Engine(float fTimeDelta)
{
    m_pInputManager->Update(fTimeDelta);
    m_pSceneManager->Pre_Update(fTimeDelta);
    m_pGameObjectManager->Pre_Update(0, fTimeDelta);
    m_pCameraManager->Pre_Update(fTimeDelta);
    m_pUIManager->Pre_Update(fTimeDelta);
}

void CGameManager::Update_Engine(float fTimeDelta)
{
    m_pSceneManager->Update(fTimeDelta);
    m_pGameObjectManager->Update(0, fTimeDelta);
    m_pCameraManager->Update(fTimeDelta);
    m_pUIManager->Update(fTimeDelta);
    m_pSoundManager->Update(fTimeDelta);
}

void CGameManager::Late_Update_Engine(float fTimeDelta)
{
    m_pSceneManager->Late_Update(fTimeDelta);
    m_pGameObjectManager->Late_Update(0, fTimeDelta);
    m_pCameraManager->Late_Update(fTimeDelta);
    m_pUIManager->Late_Update(fTimeDelta);
    m_pGameObjectManager->Add_RenderGroup(Engine::GLOBAL_LEVEL);
    m_pSceneManager->Add_RenderGroup();
    m_pGameObjectManager->Add_ColliderGroup(Engine::GLOBAL_LEVEL);
    const uint32 iCurrentSceneID = m_pSceneManager->GetCurSceneID();
    if (iCurrentSceneID != Engine::GLOBAL_LEVEL)
        m_pGameObjectManager->Add_ColliderGroup(iCurrentSceneID);
    m_pCollisionManager->Update(fTimeDelta);
    m_pCollisionManager->Clean_DeadCollision();
    m_pGameObjectManager->Clean_DeadObject();
    m_pSceneManager->ExecutePendingSceneChanges();
}

void CGameManager::Draw(CBase* pRenderObject)
{
    if (m_pGraphicDevice == nullptr)
        return;

    m_pGraphicDevice->Begin_Render();
    m_pCameraManager->Render();
    m_pRenderManager->RenderGroup();
    CComponent* pRenderComponent = static_cast<CComponent*>(pRenderObject);
    if (pRenderComponent != nullptr && pRenderComponent->Get_IsActive())
        pRenderComponent->Render();
    m_pSceneManager->Render();
    m_pUIManager->Render();
    if (m_pDebugOverlay != nullptr)
    {
        m_pDebugOverlay->Render_FPS(
            TimerManger_Get_FPS(L"Timer_Default"),
            m_pGraphicDevice->Get_Width(),
            m_pGraphicDevice->Get_Height());
    }
    m_pScreenShotManager->ExecutePending_ScreenShot();
    m_pGraphicDevice->End_Render();
    m_pRenderManager->Clean_RenderGroup();
}

void CGameManager::Resize_Engine(uint32 iWinSizeX, uint32 iWinSizeY)
{
    if (m_pGraphicDevice != nullptr)
    {
        m_pGraphicDevice->Resize(iWinSizeX, iWinSizeY);
#ifndef __ANDROID__
        iWinSizeX = m_pGraphicDevice->Get_Width();
        iWinSizeY = m_pGraphicDevice->Get_Height();
#else
        // Android는 Surface 회전/PreTransform 때문에 SwapChain Desc와
        // 실제 NativeWindow 표시 크기가 다르게 들어올 수 있다.
        // 카메라/뷰포트는 화면에 보이는 NativeWindow 크기를 기준으로 맞춘다.
        if (iWinSizeX == 0 || iWinSizeY == 0)
        {
            iWinSizeX = m_pGraphicDevice->Get_Width();
            iWinSizeY = m_pGraphicDevice->Get_Height();
        }
#endif
    }

    if (m_pCameraManager != nullptr && iWinSizeX != 0 && iWinSizeY != 0)
        m_pCameraManager->Resize(static_cast<float>(iWinSizeX), static_cast<float>(iWinSizeY));
}

void CGameManager::Release_Engine()
{
    Free();
    DestroyInstance();
}

HRESULT CGameManager::TimerManager_Add_Timer(const std::wstring& strKey)
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Add_Timer(strKey) : E_FAIL;
}

float CGameManager::TimerManager_Get_TimeDelta(const std::wstring& strKey) const
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Get_TimeDelta(strKey) : 0.f;
}

void CGameManager::TimerManager_Compute_TimeDelta(const std::wstring& strKey)
{
    if (m_pTimerManager != nullptr)
        m_pTimerManager->Compute_TimeDelta(strKey);
}

uint32 CGameManager::TimerManger_Get_FPS(const std::wstring& strKey) const
{
    return m_pTimerManager != nullptr ? m_pTimerManager->Get_FPS(strKey) : 0;
}

HRESULT CGameManager::PrototypeManager_Add_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                                      uint32 iLevelID,
                                                      const std::wstring& strPrototypeTag,
                                                      CBase* pPrototype)
{
    return m_pPrototypeManager != nullptr
        ? m_pPrototypeManager->Add_Prototype(ePrototypeType, iLevelID, strPrototypeTag, pPrototype)
        : E_FAIL;
}

CBase* CGameManager::PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                                       uint32 iLevelID,
                                                       const std::wstring& strPrototypeTag,
                                                       void* pArg) const
{
    return m_pPrototypeManager != nullptr
        ? m_pPrototypeManager->Clone_Prototype(ePrototypeType, iLevelID, strPrototypeTag, pArg)
        : nullptr;
}

void CGameManager::PrototypeManager_Clear_Prototype(uint32 iLevelID)
{
    if (m_pPrototypeManager != nullptr)
        m_pPrototypeManager->Clear_Prototype(iLevelID);
}

HRESULT CGameManager::ResourceManager_Load_Texture(const std::wstring& strKey,
                                                    const std::wstring& strPath,
                                                    uint32 iTextureCount)
{
    if (m_pResourceManager == nullptr || m_pGraphicDevice == nullptr ||
        strKey.empty() || strPath.empty() || iTextureCount == 0)
        return E_INVALIDARG;

    // 媛숈? ?ㅺ? ?대? ?덉쑝硫?Scene ?ъ쭊?????ㅼ떆 GPU Texture瑜?留뚮뱾吏 ?딅뒗??
    CResource* pExisting = m_pResourceManager->Get_Resource(strKey, RESOURCE_TYPE::TEXTURE, false);
    if (pExisting != nullptr)
    {
        pExisting->Release();
        return S_OK;
    }

    CTexture* pTexture = CTexture::Create(
        strPath,
        iTextureCount,
        m_pGraphicDevice->Get_Device(),
        m_pGraphicDevice->Get_DeviceContext());
    if (pTexture == nullptr)
        return E_FAIL;

    const HRESULT hr = m_pResourceManager->Add_Resource(strKey, pTexture);
    if (FAILED(hr))
        pTexture->Release();
    return hr;
}

HRESULT CGameManager::ResourceManager_Add_Resource(const std::wstring& strKey, CResource* pResource)
{
    return m_pResourceManager != nullptr
        ? m_pResourceManager->Add_Resource(strKey, pResource)
        : E_FAIL;
}

CResource* CGameManager::ResourceManager_Get_Resource(const std::wstring& strKey,
                                                       RESOURCE_TYPE eResourceType,
                                                       bool isCopy,
                                                       void* pArg) const
{
    return m_pResourceManager != nullptr
        ? m_pResourceManager->Get_Resource(strKey, eResourceType, isCopy, pArg)
        : nullptr;
}

HRESULT CGameManager::ResourceManager_Remove_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType)
{
    return m_pResourceManager != nullptr
        ? m_pResourceManager->Remove_Resource(strKey, eResourceType)
        : E_FAIL;
}

CGameObject* CGameManager::GameObjectManager_Add_GameObject(uint32 iPrototypeLevelID,
                                                             const std::wstring& strPrototypeTag,
                                                             uint32 iCloneLevelID,
                                                             const std::wstring& strLayerTag,
                                                             void* pArg)
{
    return m_pGameObjectManager != nullptr
        ? m_pGameObjectManager->Add_GameObject(iPrototypeLevelID, strPrototypeTag, iCloneLevelID, strLayerTag, pArg)
        : nullptr;
}

CGameObject* CGameManager::GameObjectManager_Find_GameObject(uint32 iLevelID,
                                                              const std::wstring& strLayerTag,
                                                              const std::wstring& strGameObjectName) const
{
    return m_pGameObjectManager != nullptr
        ? m_pGameObjectManager->Find_GameObject(iLevelID, strLayerTag, strGameObjectName)
        : nullptr;
}

void CGameManager::GameObjectManager_Pre_Update(uint32 iLevelID, float fTimeDelta) { if(m_pGameObjectManager)m_pGameObjectManager->Pre_Update(iLevelID,fTimeDelta); }
void CGameManager::GameObjectManager_Update(uint32 iLevelID, float fTimeDelta) { if(m_pGameObjectManager)m_pGameObjectManager->Update(iLevelID,fTimeDelta); }
void CGameManager::GameObjectManager_Late_Update(uint32 iLevelID, float fTimeDelta) { if(m_pGameObjectManager)m_pGameObjectManager->Late_Update(iLevelID,fTimeDelta); }
HRESULT CGameManager::GameObjectManager_Add_RenderGroup(uint32 iLevelID) { return m_pGameObjectManager?m_pGameObjectManager->Add_RenderGroup(iLevelID):E_FAIL; }
void CGameManager::GameObjectManager_Add_ColliderGroup(uint32 iLevelID) { if(m_pGameObjectManager)m_pGameObjectManager->Add_ColliderGroup(iLevelID); }
HRESULT CGameManager::GameObjectManager_Clear(uint32 iLevelID) { return m_pGameObjectManager?m_pGameObjectManager->Clear_GameObjectByLevelID(iLevelID):E_FAIL; }

HRESULT CGameManager::RenderManager_Add_RenderGroup(RENDER_GROUP eRenderGroup, CGameObject* pGameObject)
{ return m_pRenderManager?m_pRenderManager->Add_RenderGroup(eRenderGroup,pGameObject):E_FAIL; }

HRESULT CGameManager::SceneManager_Register_Factory(uint32 iSceneID, const std::function<CScene*(const Engine::CREATE_SCENE_DESC&)>& Factory)
{ return m_pSceneManager?m_pSceneManager->Register_SceneFactory(iSceneID,Factory):E_FAIL; }
void CGameManager::SceneManager_Request_Change(const Engine::CREATE_SCENE_DESC& Desc)
{ if(m_pSceneManager)m_pSceneManager->RequestChangeScene(Desc); }
uint32 CGameManager::SceneManager_Get_CurrentSceneID() const
{ return m_pSceneManager?m_pSceneManager->GetCurSceneID():Engine::GLOBAL_LEVEL; }

bool CGameManager::InputManager_GetButtonPress(KEY_TYPE eKey) const { return m_pInputManager&&m_pInputManager->GetButtonPress(eKey); }
bool CGameManager::InputManager_GetButtonDown(KEY_TYPE eKey) const { return m_pInputManager&&m_pInputManager->GetButtonDown(eKey); }
bool CGameManager::InputManager_GetButtonUp(KEY_TYPE eKey) const { return m_pInputManager&&m_pInputManager->GetButtonUp(eKey); }
const Engine::TOUCH_STATE& CGameManager::InputManager_GetTouchState(uint32 iTouchIndex) const
{
    static const Engine::TOUCH_STATE Empty{};
    return m_pInputManager?m_pInputManager->GetTouchState(iTouchIndex):Empty;
}
void CGameManager::InputManager_SetNativeTouchState(uint32 iTouchIndex,bool isPressed,float fX,float fY)
{if(m_pInputManager)m_pInputManager->Set_NativeTouchState(iTouchIndex,isPressed,fX,fY);}
HRESULT CGameManager::CollisionManager_Register_CollisionTable(uint32 a,uint32 b){return m_pCollisionManager?m_pCollisionManager->Register_CollisionTable(a,b):E_FAIL;}
HRESULT CGameManager::CollisionManager_Add_Collider(uint32 g,CCollider* c){return m_pCollisionManager?m_pCollisionManager->Add_Collider(g,c):E_FAIL;}
void CGameManager::CollisionManager_Clear_Collision(){if(m_pCollisionManager)m_pCollisionManager->Clear_Collision();}
HRESULT CGameManager::UIManager_Add_Canvas(uint32 i,CGameObject* o){return m_pUIManager?m_pUIManager->Add_Canvas(i,o):E_FAIL;}
CGameObject* CGameManager::UIManager_Get_Canvas(uint32 i)const{return m_pUIManager?m_pUIManager->Get_Canvas(i):nullptr;}
void CGameManager::UIManager_Release_Canvas(uint32 i){if(m_pUIManager)m_pUIManager->Release_Canvas(i);}
void CGameManager::UIManager_Fade_In(float t){if(m_pUIManager)m_pUIManager->Fade_In(t);}
void CGameManager::UIManager_Fade_Out(float t){if(m_pUIManager)m_pUIManager->Fade_Out(t);}
HRESULT CGameManager::CameraManager_Add_Camera(const std::wstring& k,UPDATE_MODE m,CCameraObjectBase* o){return m_pCameraManager?m_pCameraManager->Add_Camera(k,m,o):E_FAIL;}
HRESULT CGameManager::CameraManager_Remove_Camera(const std::wstring& k,UPDATE_MODE m){return m_pCameraManager?m_pCameraManager->Remove_Camera(k,m):E_FAIL;}
CCameraObjectBase* CGameManager::CameraManager_Get_Camera(const std::wstring& k,UPDATE_MODE m)const{return m_pCameraManager?m_pCameraManager->Get_Camera(k,m):nullptr;}
CCameraObjectBase* CGameManager::CameraManager_Get_MainCamera()const{return m_pCameraManager?m_pCameraManager->Get_MainCamera():nullptr;}
HRESULT CGameManager::CameraManager_Switch_MainCamera(const std::wstring& k,UPDATE_MODE m){return m_pCameraManager?m_pCameraManager->Switch_MainCamera(k,m):E_FAIL;}
HRESULT CGameManager::PlayerManager_Add_Player(const std::wstring& k,CGameObject* o){return m_pPlayerManager?m_pPlayerManager->Add_Player(k,o):E_FAIL;}
CGameObject* CGameManager::PlayerManager_Get_Player(const std::wstring& k)const{return m_pPlayerManager?m_pPlayerManager->Get_Player(k):nullptr;}
HRESULT CGameManager::LightManager_Add_Light(const Engine::LIGHT_DESC& d){return m_pLightManager?m_pLightManager->Add_Light(d):E_FAIL;}
CLight* CGameManager::LightManager_Get_Light(LIGHT_TYPE t,uint32 i)const{return m_pLightManager?m_pLightManager->Get_Light(t,i):nullptr;}
HRESULT CGameManager::CellManager_Add_Cells(const std::wstring& k,const std::vector<CCell*>& c){return m_pCellManager?m_pCellManager->Add_Cells(k,c):E_FAIL;}
const std::vector<CCell*>& CGameManager::CellManager_Get_Cells(const std::wstring& k)const{static const std::vector<CCell*> Empty;return m_pCellManager?m_pCellManager->Get_Cells(k):Empty;}
HRESULT CGameManager::RenderStateManager_Add_RenderState(uint32 i,const Engine::RENDER_STATE_DESC& d){return m_pRenderStateManager?m_pRenderStateManager->Add_RenderState(i,d):E_FAIL;}
const Engine::RENDER_STATE_DESC* CGameManager::RenderStateManager_Get_RenderState(uint32 i)const{return m_pRenderStateManager?m_pRenderStateManager->Get_RenderState(i):nullptr;}
HRESULT CGameManager::RenderTargetManager_Add_RenderTarget(const std::wstring& k,uint32 w,uint32 h,const float* c){return m_pRenderTargetManager?m_pRenderTargetManager->Add_RenderTarget(k,w,h,c):E_FAIL;}
HRESULT CGameManager::RenderTargetManager_Add_MRT(const std::wstring& m,const std::wstring& t){return m_pRenderTargetManager?m_pRenderTargetManager->Add_MRT(m,t):E_FAIL;}
HRESULT CGameManager::RenderTargetManager_Begin_MRT(const std::wstring& k){return m_pRenderTargetManager?m_pRenderTargetManager->Begin_MRT(k):E_FAIL;}
HRESULT CGameManager::RenderTargetManager_End_MRT(){return m_pRenderTargetManager?m_pRenderTargetManager->End_MRT():E_FAIL;}
HRESULT CGameManager::RenderTargetManager_Bind_ShaderResource(const std::wstring& k,const std::string& v,CShader* s){return m_pRenderTargetManager?m_pRenderTargetManager->Bind_ShaderResource(k,v,s):E_FAIL;}
HRESULT CGameManager::FontMananger_Add_Font(float s,const std::wstring& k,const std::wstring& p){return m_pFontManager?m_pFontManager->Add_Font(s,k,p):E_FAIL;}
HRESULT CGameManager::FontMananger_Draw(const std::wstring& k,const std::wstring& t,float x,float y,const float c[4])const{return m_pFontManager?m_pFontManager->Draw(k,t,x,y,c):E_FAIL;}
HRESULT CGameManager::SoundManager_Add_Sound(const std::wstring& k,const std::wstring& p){return m_pSoundManager?m_pSoundManager->Add_Sound(k,p):E_FAIL;}
HRESULT CGameManager::SoundManager_Play(const std::wstring& k,uint32 c,float v,bool l){return m_pSoundManager?m_pSoundManager->SoundPlay(k,c,v,l):E_FAIL;}
void CGameManager::SoundManager_Stop(uint32 c){if(m_pSoundManager)m_pSoundManager->SoundStop(c);}
void CGameManager::ScreenShotManager_Request_ScreenShot(bool o,const std::wstring& p){if(m_pScreenShotManager)m_pScreenShotManager->Request_ScreenShot(o,p);}

void CGameManager::Release_Manager()
{
    if (m_pSoundManager != nullptr)
        m_pSoundManager->Free();
    CSoundManager::DestroyInstance();
    m_pSoundManager = nullptr;

    if(m_pDebugOverlay)m_pDebugOverlay->Release(); m_pDebugOverlay = nullptr;
    if(m_pFontManager)m_pFontManager->Release(); m_pFontManager = nullptr;
    if(m_pUIManager)m_pUIManager->Release(); m_pUIManager = nullptr;
    if(m_pCollisionManager)m_pCollisionManager->Release(); m_pCollisionManager = nullptr;
    if(m_pPlayerManager)m_pPlayerManager->Release(); m_pPlayerManager = nullptr;
    if(m_pCellManager)m_pCellManager->Release(); m_pCellManager = nullptr;
    if(m_pLightManager)m_pLightManager->Release(); m_pLightManager = nullptr;
    if(m_pResourceManager)m_pResourceManager->Release(); m_pResourceManager = nullptr;
    if(m_pScreenShotManager)m_pScreenShotManager->Release(); m_pScreenShotManager = nullptr;
    if(m_pCameraManager)m_pCameraManager->Release(); m_pCameraManager = nullptr;
    if(m_pRenderTargetManager)m_pRenderTargetManager->Release(); m_pRenderTargetManager = nullptr;
    if(m_pRenderStateManager)m_pRenderStateManager->Release(); m_pRenderStateManager = nullptr;
    if(m_pRenderManager)m_pRenderManager->Release(); m_pRenderManager = nullptr;
    if(m_pInputManager)m_pInputManager->Release(); m_pInputManager = nullptr;
    delete m_pPrototypeManager; m_pPrototypeManager = nullptr;
    if(m_pGameObjectManager)m_pGameObjectManager->Release(); m_pGameObjectManager = nullptr;
    if(m_pSceneManager)m_pSceneManager->Release(); m_pSceneManager = nullptr;
    if(m_pTimerManager)m_pTimerManager->Release(); m_pTimerManager = nullptr;
}

void CGameManager::Free()
{
    Release_Manager();
    delete m_pGraphicDevice;
    m_pGraphicDevice = nullptr;
    CBase::Free();
}


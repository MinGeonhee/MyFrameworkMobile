#pragma once

#include "CBase.h"
#include "Engine_Struct.h"
#include <functional>

class CCameraManager;
class CCellManager;
class CCollisionManager;
class CFontManager;
class CGameObjectManager;
class CGraphicDevice;
class CInputManager;
class CLightManager;
class CPlayerManager;
class CPrototypeManager;
class CRenderManager;
class CRenderStateManager;
class CRenderTargetManager;
class CResourceManager;
class CResource;
class CSceneManager;
class CScreenShotManager;
class CSoundManager;
class CTimeManger;
class CUIManager;
class CGameObject;
class CScene;
class CCollider;
class CLight;
class CCell;
class CShader;
class CCameraObjectBase;
class CDebugOverlay;

class ENGINE_DLL CGameManager final : public CBase
{
    DECLARE_SINGLETON(CGameManager)

private:
    CGameManager();
    ~CGameManager() override = default;

public:
    HRESULT Initialize_Engine(const Engine::Engine_Desc& tEngineDesc);
    void    Pre_Update_Engine(float fTimeDelta);
    void    Update_Engine(float fTimeDelta);
    void    Late_Update_Engine(float fTimeDelta);
    void    Draw(CBase* pRenderObject = nullptr);
    void    Resize_Engine(uint32 iWinSizeX, uint32 iWinSizeY);
    void    Release_Manager();
    void    Release_Engine();

#pragma region TimerManager
    HRESULT     TimerManager_Add_Timer(const std::wstring& strKey);
    float       TimerManager_Get_TimeDelta(const std::wstring& strKey) const;
    void        TimerManager_Compute_TimeDelta(const std::wstring& strKey);
    uint32      TimerManger_Get_FPS(const std::wstring& strKey) const;
#pragma endregion

#pragma region InputManager
    bool InputManager_GetButtonPress(KEY_TYPE eKey) const;
    bool InputManager_GetButtonDown(KEY_TYPE eKey) const;
    bool InputManager_GetButtonUp(KEY_TYPE eKey) const;
    const Engine::TOUCH_STATE& InputManager_GetTouchState(uint32 iTouchIndex) const;
    void InputManager_SetNativeTouchState(uint32 iTouchIndex,bool isPressed,float fX,float fY);
#pragma endregion

#pragma region CollisionManager
    HRESULT CollisionManager_Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID);
    HRESULT CollisionManager_Add_Collider(uint32 iGroupID, CCollider* pCollider);
    void CollisionManager_Clear_Collision();
#pragma endregion

#pragma region UIManager
    HRESULT UIManager_Add_Canvas(uint32 iIndex, CGameObject* pCanvas);
    CGameObject* UIManager_Get_Canvas(uint32 iIndex) const;
    void UIManager_Release_Canvas(uint32 iIndex);
    void UIManager_Fade_In(float fTime); void UIManager_Fade_Out(float fTime);
#pragma endregion

#pragma region CameraPlayerManager
    HRESULT CameraManager_Add_Camera(const std::wstring& strKey, UPDATE_MODE eMode, CCameraObjectBase* pCamera);
    HRESULT CameraManager_Remove_Camera(const std::wstring& strKey, UPDATE_MODE eMode);
    CCameraObjectBase* CameraManager_Get_Camera(const std::wstring& strKey, UPDATE_MODE eMode) const;
    CCameraObjectBase* CameraManager_Get_MainCamera() const;
    HRESULT CameraManager_Switch_MainCamera(const std::wstring& strKey, UPDATE_MODE eMode);
    HRESULT PlayerManager_Add_Player(const std::wstring& strKey, CGameObject* pPlayer);
    CGameObject* PlayerManager_Get_Player(const std::wstring& strKey) const;
#pragma endregion

#pragma region LightCellManager
    HRESULT LightManager_Add_Light(const Engine::LIGHT_DESC& Desc);
    CLight* LightManager_Get_Light(LIGHT_TYPE eType, uint32 iIndex) const;
    HRESULT CellManager_Add_Cells(const std::wstring& strKey, const std::vector<CCell*>& Cells);
    const std::vector<CCell*>& CellManager_Get_Cells(const std::wstring& strKey) const;
#pragma endregion

#pragma region RenderStateTargetManager
    HRESULT RenderStateManager_Add_RenderState(uint32 iIndex, const Engine::RENDER_STATE_DESC& Desc);
    const Engine::RENDER_STATE_DESC* RenderStateManager_Get_RenderState(uint32 iIndex) const;
    HRESULT RenderTargetManager_Add_RenderTarget(const std::wstring& strKey,uint32 iWidth,uint32 iHeight,const float* pClearColor);
    HRESULT RenderTargetManager_Add_MRT(const std::wstring& strMRTKey,const std::wstring& strTargetKey);
    HRESULT RenderTargetManager_Begin_MRT(const std::wstring& strKey); HRESULT RenderTargetManager_End_MRT();
    HRESULT RenderTargetManager_Bind_ShaderResource(const std::wstring& strTargetKey,const std::string& strVariable,CShader* pShader);
#pragma endregion

#pragma region FontSoundScreenShotManager
    HRESULT FontMananger_Add_Font(float fSize,const std::wstring& strKey,const std::wstring& strPath);
    HRESULT FontMananger_Draw(const std::wstring& strKey,const std::wstring& strText,float x,float y,const float color[4]) const;
    HRESULT SoundManager_Add_Sound(const std::wstring& strKey,const std::wstring& strPath);
    HRESULT SoundManager_Play(const std::wstring& strKey,uint32 iChannel,float fVolume,bool isLoop=false);
    void SoundManager_Stop(uint32 iChannel);
    void ScreenShotManager_Request_ScreenShot(bool isOverlay,const std::wstring& strFilePath=L"");
#pragma endregion

#pragma region GameObjectManager
    CGameObject* GameObjectManager_Add_GameObject(uint32 iPrototypeLevelID,
                                                   const std::wstring& strPrototypeTag,
                                                   uint32 iCloneLevelID,
                                                   const std::wstring& strLayerTag,
                                                   void* pArg = nullptr);
    CGameObject* GameObjectManager_Find_GameObject(uint32 iLevelID,
                                                   const std::wstring& strLayerTag,
                                                   const std::wstring& strGameObjectName) const;
    void GameObjectManager_Pre_Update(uint32 iLevelID, float fTimeDelta);
    void GameObjectManager_Update(uint32 iLevelID, float fTimeDelta);
    void GameObjectManager_Late_Update(uint32 iLevelID, float fTimeDelta);
    HRESULT GameObjectManager_Add_RenderGroup(uint32 iLevelID);
    void GameObjectManager_Add_ColliderGroup(uint32 iLevelID);
    HRESULT GameObjectManager_Clear(uint32 iLevelID);
#pragma endregion

#pragma region RenderManager
    HRESULT RenderManager_Add_RenderGroup(RENDER_GROUP eRenderGroup, CGameObject* pGameObject);
#pragma endregion

#pragma region SceneManager
    HRESULT SceneManager_Register_Factory(uint32 iSceneID, const std::function<CScene*(const Engine::CREATE_SCENE_DESC&)>& Factory);
    void SceneManager_Request_Change(const Engine::CREATE_SCENE_DESC& Desc);
    uint32 SceneManager_Get_CurrentSceneID() const;
#pragma endregion

#pragma region ResourceManager
    HRESULT ResourceManager_Load_Texture(const std::wstring& strKey,
                                         const std::wstring& strPath,
                                         uint32 iTextureCount = 1);
    HRESULT ResourceManager_Add_Resource(const std::wstring& strKey, CResource* pResource);
    CResource* ResourceManager_Get_Resource(const std::wstring& strKey,
                                            RESOURCE_TYPE eResourceType,
                                            bool isCopy = false,
                                            void* pArg = nullptr) const;
    HRESULT ResourceManager_Remove_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType);
#pragma endregion

#pragma region PrototypeManager
    HRESULT PrototypeManager_Add_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                           uint32 iLevelID,
                                           const std::wstring& strPrototypeTag,
                                           CBase* pPrototype);
    CBase* PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                            uint32 iLevelID,
                                            const std::wstring& strPrototypeTag,
                                            void* pArg = nullptr) const;
    void PrototypeManager_Clear_Prototype(uint32 iLevelID);
#pragma endregion

private:
    HRESULT Initialize_Manager(const Engine::Engine_Desc& tEngineDesc);
private:
    CGraphicDevice*       m_pGraphicDevice = nullptr;
    CTimeManger*          m_pTimerManager = nullptr;
    CSceneManager*        m_pSceneManager = nullptr;
    CGameObjectManager*   m_pGameObjectManager = nullptr;
    CPrototypeManager*    m_pPrototypeManager = nullptr;
    CInputManager*        m_pInputManager = nullptr;
    CRenderManager*       m_pRenderManager = nullptr;
    CRenderStateManager*  m_pRenderStateManager = nullptr;
    CRenderTargetManager* m_pRenderTargetManager = nullptr;
    CCameraManager*       m_pCameraManager = nullptr;
    CScreenShotManager*   m_pScreenShotManager = nullptr;
    CResourceManager*     m_pResourceManager = nullptr;
    CLightManager*        m_pLightManager = nullptr;
    CCellManager*         m_pCellManager = nullptr;
    CPlayerManager*       m_pPlayerManager = nullptr;
    CCollisionManager*    m_pCollisionManager = nullptr;
    CUIManager*           m_pUIManager = nullptr;
    CFontManager*         m_pFontManager = nullptr;
    CSoundManager*        m_pSoundManager = nullptr;
    CDebugOverlay*        m_pDebugOverlay = nullptr;
private:
    void Free() override;
};

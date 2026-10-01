#pragma once
#include "CBase.h"
#include "CCameraManager.h"
#include "CEventBus.h"
#include "CLightManager.h"


NS_BEGIN(Engine)

FD      CTimeManger;
FD      CSceneManager;
FD      CGraphicDevice;
FD      CPrototypeManager;
FD      CGameObjectManager;
FD      CUIManager;
FD      CPlayerManager;
FD      CRenderManager;
FD      CRenderTargetManager;
FD      CRenderTarget;
FD      CCameraManager;
FD      CCameraObjectBase;
FD      CInputManager;
FD      CScreenShotManager;
FD      CRenderStateManager;
FD      CResourceManager;
FD      CCellManager;
FD      CFontManager;
FD      CFont;
FD      CCollisionManager;
FD      CGameObject;
FD      CLayer;
FD      CShader;
FD      CScreenShot;
FD      CCanvas;
FD      CScene;
FD      CCell;
FD      CResource;
FD      CPipeLine;
FD      CVIBuffer_Rect;

using SceneFactory = std::function<CScene* (const CreateScene_Desc&)>;
using RendereGroupSortFunc = std::function<bool(CONST CGameObject*, CONST CGameObject*)>;


class ENGINE_DLL CGameManager :    public CBase
{
    DECLARE_SINGLETON(CGameManager)
private:
    explicit CGameManager();
    virtual ~CGameManager() = default;
public:
    HRESULT             Initialize_Engine(const Engine_Desc& tEngineDesc, ComPtr<Device>* ppDevice, ComPtr<DeviceContext>* ppDeviceContext);
    void                Pre_Update_Engine(float DT);
    void                Update_Engine(float DT);
    void                Late_Update_Engine(float DT);
    void                Add_RenderGroup();

    
    HRESULT             EndFrame();

    HRESULT             Draw();

public:

    int32               Get_RandomInt(int32 iMin , int32 iMax);
    float               Get_Randomfloat(float fMin, float fMax);
    Float4              Get_RandomPosArea(const fVector4_1& vCenter , float minRange , float maxRange );
    Float3              Get_RandomPosition(Float3 fRange,Float3 vCenter);
    ShaderResourceView* Make_ShaderResourceViewColor(uint32 A, uint32 R, uint32 G, uint32 B , ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
public:

    CONST uint32&       Get_DefaultWinSizeX() CONST{ return m_iDefaultWinSizeX; }
    CONST uint32&       Get_DefaultWinSizeY() CONST{ return m_iDefaultWinSizeY; }


public:
    void                Set_IsDebug(bool isDebug) { m_isDebugMode = isDebug; }
    bool                Get_IsDebug() { return m_isDebugMode ; }

    HRESULT             Reaydy_Default(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);

public:
    void                Release_Manager();
public:

#pragma region GameManager
    void                ChangeScene_Clear_CurSceneObject(uint32 iCurSceneID);
#pragma endregion

#pragma region TimerManager
    HRESULT	            TimerManager_Add_Timer(const wstring strKey); // Timer 추가
    float	            TimerManager_Get_TimeDelta(const wstring strKey); // TImer DT 가져오기
    void	            TimerManager_Compute_TimeDelta(const wstring strKey); // Timer 누적시키기
    uint32              TimerManger_Get_FPS(const wstring strKey);

#pragma endregion

#pragma region GraphicDevice;

    void                        GraphicDevice_RenderBegin(const Color& ClearColor);
    void                        GraphicDevice_RenderEnd();

    HRESULT                     GraphicDevice_Get_BackBuffer(ComPtr<Texture2D>* pTexture2D, uint32 iFlag = 0);
    /* 창 / Surface 크기 변경 (엔진 렌더 해상도는 그대로 두고 화면 출력만 맞춘다) */
    void                        GraphicDevice_Resize(uint32 iWinSizeX, uint32 iWinSizeY);

    HWND                        GraphicDevice_Get_HWND();
    HANDLE                      GraphicDevice_Get_Handle();

    ComPtr<Device>              GraphicDevice_Get_Device();
    ComPtr<DeviceContext>       GraphicDevice_Get_DeviceContext();

    ComPtr<RTV>                 GraphicDevice_Get_RTV();
    ComPtr<DepthStencilView>    GraphicDevice_Get_DSV();

    HRESULT                     GraphicDevice_Set_GPU_OriginViewPort();

    const VIEWPORT_DESC&       GraphicDevice_Get_OriginViewPort();
    void                        CGraphicDevice_Set_ScissorRectsOrigin();



#pragma endregion 

#pragma region ResourceManager
    CResource*                  ResourceManager_Get_Resource(const wstring& wstrKey, RESOURCE_TYPE eResourceType, bool isCopy, void* pArg = nullptr);
    HRESULT                     ResourceManager_Add_Resource(const wstring& wstrKey, CResource* pResource);
#pragma endregion

#pragma region SceneManager
    
    HRESULT		                SceneManager_Register_SceneFactory(uint32 iSceneID, const SceneFactory& SceneCreateFuntion);
    void		                SceneManager_RequestChangeScene(const CreateScene_Desc& tDesc);
    HRESULT			            SceneManager_ExecutePendingSceneChanges();
    vector<CScene*>&            SceneManager_GetStackScene();

    uint32				        SceneManager_GetCurrentSceneID();
    CScene*                     SceneManager_GetCurScene();


#pragma endregion

#pragma region Cell Manager

    const vector<CCell*>&     CellManager_Get_Cells(const wstring& wstrFindKey);
    HRESULT                   CellManager_Add_Cells(const wstring& wstrCellPath, const wstring& wstrAddKey);

    bool				      CellManager_Get_IsCellRender() const;
    void				      CellManager_Set_IsCellRender(bool isRender);

#pragma endregion


#pragma region Collision Manager

    HRESULT	            CollisionManager_Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID);
    HRESULT             CollisionManager_Add_Collider(uint32 iAddGroupID, CCollider* pCollider);
    void	            CollisionManager_RayCollision(uint32 iColliderGruopID, const RAY_INFO& tRayInfo, RayHit* pRayHit);
    void	            CollisionManager_Clear_Collision();

#pragma endregion

#pragma region PrototypeManager
    CBase*           PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE eCloneProtoType, uint32 iCloneProtoSceneID, const wstring& strCloneProtoKey, void* pArg);
    CBase*           PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE eCloneProtoType, const wstring& strCloneProtoKey, void* pArg);

    void            Clear_Prototype_CurSceneID(uint32 iCurSceneID);
    HRESULT         PrototypeManager_Add_Prototype(PROTOTYPE_TYPE eAddProtoType, uint32 iAddProtoSceneID, const wstring& strAddProtoKey, CBase* pPrototype);
    HRESULT         PrototypeManager_Add_Prototype_InGlobal(PROTOTYPE_TYPE eProtoType, const wstring& strAddProtoKey, CBase* pPrototype);






#ifdef _DEBUG
    /* PrototypeManager  */
    map<const wstring, CBase*>&         PrototypeManager_GetGameObjectPrototypeList_ByScene(uint32 iSceneID);
    map<const wstring, CBase*>&         PrototypeManager_GetComponentPrototypeList_ByScene(uint32 iSceneID);
    const map<const wstring, CBase*>&   PrototypeManager_GetGlobalGameObjectPrototypeList();
    const map<const wstring, CBase*>&   PrototypeManager_GetGlobalComponentPrototypeList();

#endif // _DEBUG

#pragma endregion


#pragma region UI Manager
    HRESULT		UIManager_Add_Canvas(uint32 iIndex, CCanvas* pCanvas);
    CCanvas*    UIManager_Get_Canvas(uint32 iIndex);
    void        UIManager_Release_Canvas(uint32 iIndex);
    void		UIManager_Fade_Out(float fTime);
    void		UIManager_Fade_In(float fTime);

#pragma endregion

#pragma region GameObjectManager
    CGameObject*        GameObjectManager_Add_GameObject(uint32 iFindProtoSceneID, const wstring& strCloneProtokey, uint32 iAddCloneSceneID, const wstring& strAddLayerKey, void* pArg);
    CGameObject*        GameObjectManager_Add_GameObject_ByGlobalProto(const wstring& strFindProtoKey, const wstring& strAddLayerKey, void* pArg = nullptr, uint32 iAddCloneSceneID = GLOBAL);
    void                GameObjectManager_Pre_Update(uint32 iSceneID, float DT);
    void                GameObjectManager_Update(uint32 iSceneID, float DT);
    void                GameObjectManager_Late_Update(uint32 iSceneID, float DT);
    HRESULT             GameObjectManager_Add_RenderGroup(uint32 iSceneID);
    HRESULT             GameObjectManager_Clear_GameObjectByScneeID(uint32 iSceneID);

    const CLayer*       GameObjectManager_Get_Layer(uint32 iSceneID, const wstring& wstrLayerName);


    CGameObject*        GameObjectManager_Find_GameObject_InGlobal(const wstring& strFindLayerKey, const wstring& strGameObjectName);
    CGameObject*        GameObjectManager_Find_GameObject_InGlobal(const wstring& strFindLayerKey, CGameObject* pFindGameObject);

    CGameObject*        GameObjectManager_Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, CGameObject* pFindGameObject);
    CGameObject*        GameObjectManager_Find_GameObject(uint32 iFindSceneID, const wstring& strFindLayerKey, const wstring& strGameObjectName);

    void                GameObjectManager_Add_ColliderGroup(uint32 iSceneID);


#ifdef _DEBUG

    const map<const wstring, CLayer*>&          GameObjectManager_GetCurSceneLayer(uint32 iCurSceneID);
    const vector<map<const wstring, CLayer*>>&  GameObjectManager_Get_AllSceneLayer();
    const map<const wstring, CLayer*>&          GameObjectManager_GetGlobalLayer();
#endif


#pragma endregion

#pragma region RenderManager

    HRESULT RenderManager_RenderGruop(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT RenderManager_RenderGruop(CCameraObjectBase* pRenderCam);
    HRESULT RenderManager_Add_RenderGroup(RENDER_GROUP eRenderGruop, CGameObject* pGameObject);
    HRESULT RenderManager_Clean_RenderGroup();

    HRESULT RenderManager_RenderGruop_APlan(const vector<CCameraObjectBase*>& vecRenderCam);
    void    RenderManager_Set_RenderMode(int32 iMode);
    void    RenderManager_Set_BenchLabel(const string& strLabel);
    int32   RenderManager_Get_RenderMode();
    void    RenderManager_Set_PerCameraDeferred(bool isPerCamera);
    bool    RenderManager_Get_PerCameraDeferred();
    void    RenderManager_GpuTimer_Begin(uint32 iCamCount);
    void    RenderManager_GpuTimer_End();

#ifdef _DEBUG
    HRESULT RenderManager_Add_DebugComponent(CComponent* pComponent);
    HRESULT RenderManager_Render_DebugPass(const vector<CCameraObjectBase*>& vecRenderCam);
#endif
#pragma endregion

#pragma region RenderTargetManager
    HRESULT	RenderTargetManager_Add_RenderTarget(const wstring& wstrAddKey, uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat);
    HRESULT RenderTargetManager_Add_MRT(const wstring& wstrAddMRTKey, const wstring& wstrFindRenderTargetKey);

    /* Begin End 함수 */
    HRESULT	RenderTargetManager_Begin_MRT(const wstring& wstrFindMRT);
    HRESULT	RenderTargetManager_Begin_MRT_NoClear(const wstring& wstrFindMRT);
    HRESULT	RenderTargetManager_Clear_MRT_Rect(const wstring& wstrFindMRT, const RECT_DESC& tRect);
    HRESULT RenderTargetManager_End_MRT();
    HRESULT RenderTargetManager_Bind_ShaderResource(const wstring& wstrFindRenderTargetKey, const string& strBindTexName, CShader* pShader);

    ShaderResourceView* RenderTargetManager_Get_ShaderResourceView(const wstring& wstrFindRenderTargetKey);



#ifdef _DEBUG
    HRESULT	RenderTargetManager_Ready_Debug(const wstring& wstrFindRenderTargetKey, float fX, float fY, float fSizeX, float fSizeY);
    HRESULT	RenderTargetManager_Render_Debug(const wstring& wstrFindRenderTargtKey, CShader* pShader, CVIBuffer_Rect* pBuffer);
#endif // _DEBUG


#pragma endregion 

#pragma region CameraManager
public:
    HRESULT                         CameraManager_MakeDefaultCamera(CONST CDefaultCameraObject::DefaultCameraObject_Dest* pDesc);
    HRESULT							CameraManager_Add_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode, CCameraObjectBase* pCamera);
    HRESULT                         CameraManager_Remove_Camera(const wstring& wstrKey, UPDATE_MODE eUpdateMode);
    CCameraObjectBase*              CameraManager_Get_Camera(CONST wstring& wstrKey, UPDATE_MODE eUpdateMode);
    void							CameraManager_Set_CameraRenderActive(const wstring& wstrKey, UPDATE_MODE eUpdateMode, bool isRednerActive);
    const array<unordered_map<wstring, CCameraObjectBase*>, Cast_Uint(UPDATE_MODE::END) >& CameraManager_Get_CameraList();

    CCameraObjectBase*              CameraManager_Get_CurRenderingCamera();
    HRESULT                         CameraManager_MakeDefaultUICamera(const CUICameraObject::UICameraObject_Desc* pUICameraDest);
    HRESULT                         CameraManager_Render();


#ifdef _DEBUG
#endif

#pragma endregion

#pragma region Light Manager
    
    const LIGHT_GROUP_BUF*          LightManager_Get_LightGroupBuffer();
    CLight*                         LightManager_Get_Light(LIGHT_TYPE eType, uint32 iIndex);
    const LIGHT_BUF_DATA&           LightManager_Get_LightBufferData(LIGHT_TYPE eType, uint32 iIndex);
    HRESULT							LightManager_Add_Light(const CLight::Light_Desc& tDesc);
    HRESULT							LightManager_Render_Light(CShader* pSader, CVIBuffer_Rect* pVIBuffer, CCameraObjectBase* pRenderCam);
    list<CLight*>					LightManager_Get_LightList(LIGHT_TYPE eType);
    HRESULT							LightManager_Clear_Light();
    HRESULT							LightManager_Add_Light(CLight* pLight);



#pragma endregion

#pragma region PipeLine

    HRESULT                         PipeLine_Set_PipeLineMatrix(CONST std::wstring& wstrFindKey, D3DTS eTransformMatrix, const XMatrix_1& TransformMatrix);
    HRESULT                         PipeLine_Set_PipeLineMatrix(CONST std::wstring& wstrFindKey, D3DTS eTransformMatrix, const Float4x4& TransformMatrix);

    Float4x4 	                    PipeLine_Get_PipeLineMatrix(CONST std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) CONST;
    const Float4x4*                 PipeLine_Get_PipeLineMatrixPtr(const std::wstring& wstrFindKey, D3DTS eTransformMatrix, bool isInverse) const;

    Float4		                    PipeLine_Get_CameraPosition(CONST std::wstring& wstrFindKey) CONST;

    HRESULT			                PipeLine_Link_EffectConstantBuffer(GLOBAL_CB_TYPE eCBType, CShader* pShader);

    HRESULT			                PipeLine_Bind_PipeLine();

    HRESULT                         PipeLine_Set_GPU_CameraBuffer(const wstring& wstrFindKey, const CAM_SHADER_INFO& tCamInfo);
    HRESULT			                PipeLine_Set_GPU_LightGroupBuffer(const LIGHT_GROUP_BUF& tLightGroupBuf);



    HRESULT                         PipeLine_Set_GPU_MouseBuffer(const MOUSE_BUF_DATA& tData);

    CONST void                      PipeLine_Clear_PipeLine();

#ifdef _DEBUG
#endif


#pragma endregion

#pragma region ScreenShot Manager

    HRESULT		    ScreenShotManager_Add_ScreenShot(const wstring& strKey, CScreenShot* pTexture);
    CScreenShot*    ScreenShotManager_Get_ScreenShot(const wstring& strKey);
    void            ScreenShotManager_Request_ScreenShot(bool isOverLay,  CScreenShot** ppScreenShot = NULLPTR);
    HRESULT         ScreenShotManager_ScreenShot(bool isOverLay,  CScreenShot** ppScreenShot = NULLPTR);

    CScreenShot*    ScreenShotMangaer_Get_OverLayScreenShot();

#pragma endregion

#pragma region RenderStateManager

    HRESULT             RenderStateManager_Set_GPU_RenderState(uint32 iIndex, STATE_TYPE eType);
    HRESULT			    RenderStateManager_Add_RenderState(uint32 iIndex,FD CRenderState* pAddRenderState);
    FD CRenderState*    RenderStateManager_Get_RenderState(uint32 iIndex, STATE_TYPE eType);

#pragma endregion



#pragma region InputManager
    bool InputManager_GetButtonPress(KEY_TYPE key);
    bool InputManager_GetButtonDown(KEY_TYPE key);
    bool InputManager_GetButtonUp(KEY_TYPE key);
    long InputManager_Get_DIMouseMove(MOUSEMOVESTATE eMouseState);
    long InputManager_Get_DIMouseState(MOUSEKEYSTATE eMouse);
    CONST POINT& InputManager_GetPreFrameMousePos();
    CONST POINT& InputManager_GetMousePos();
    void         InputManager_Mouse_Fix();
    void         InputManager_Mouse_Fix(uint32 iStartX, uint32 iStartY, uint32 iWinSizeX, uint32 iWinSizeY);
    void         InputManager_Mouse_Set_Show(bool isShow);


    
    RAY_INFO     InputManager_Get_LayWorld(const wstring& wstrCamName, UPDATE_MODE eType, RAY_TYPE eRayType);


    void InputManager_Update(float DT);

    /* 모바일 : 터치 / 가상 키 입력 전달 (NativeActivity 입력 스레드에서 호출해도 된다) */
    void InputManager_Set_NativeTouch(uint32 iSlot, bool isPressed, float fScreenX, float fScreenY);
    void InputManager_Set_NativeKey(KEY_TYPE eKey, bool isPressed);
#pragma endregion


#pragma region PlayerManager

    HRESULT         PlayerManager_Add_Player(const wstring& AddKey , CGameObject* pPlayer);
    CGameObject*    PlayerManager_Get_Player(const wstring& FindKey);


    CGameObject*    PlayerManager_Compute_CheckRange(const fVector4_1& vMonsterPos, float fMonsterRange, CGameObject* pCurTargetPlayer = nullptr);

    void            PlayerManager_ClearPlayer();

#pragma endregion

#pragma region FontManager

    HRESULT       FontMananger_Add_Font(float fOriginFontSize,const wstring& wstrFindKey, const wstring& wstrFontPath);
    CFont*        FontMananger_Get_Font(const wstring& wstrFindKey);
    HRESULT       FontMananger_Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const fVector4_1& vColor, FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);
    HRESULT       FontMananger_Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const Float2& vSize, const fVector4_1& vColor, FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);

#pragma endregion

private:
    CTimeManger*                m_pTimerManager;
    CGraphicDevice*             m_pGraphicDevice;
    CSceneManager*              m_pSceneManager;
    CCollisionManager*          m_pCollisionManager;
    CPrototypeManager*          m_pPrototypeManager;
    CGameObjectManager*         m_pGameObjectManager;
    CUIManager*                 m_pUIManager;
    CRenderManager*             m_pRenderManger;
    CRenderTargetManager*       m_pRenderTargetManager;
    CCameraManager*             m_pCameraManager;
    CScreenShotManager*         m_pScreenShotManager;
    CPlayerManager*             m_pPlayerManager;
    CRenderStateManager*        m_pRenderStateManager;
    CPipeLine*                  m_pPipeLine;
    CInputManager*              m_pInputManager;
    CResourceManager*           m_pResourceManager;
    CCellManager*               m_pCellManager;
    CLightManager*              m_pLightManager;
    CFontManager*               m_pFontManager;
private:
    uint32                      m_iDefaultWinSizeX;
    uint32                      m_iDefaultWinSizeY;	
    uint32                      m_iTotalSceneNum;
    uint32                      m_iTotalRenderGroupNum;
private:
    bool                        m_isDebugMode;


public:
    virtual void    Free() override;
};


NS_END


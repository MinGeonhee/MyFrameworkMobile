#pragma once
#include "CBase.h"

namespace Diligent { struct IQuery; }

NS_BEGIN(Engine)

FD      CGameManager;
FD      CGameObject;
FD      CLayer;
FD      CCameraObjectBase;
FD      CRenderState;
FD      CVIBuffer_Rect;
FD      CComponent;
FD      CShader;

using RenderGroup = std::vector<CGameObject*>;
using RendereGroupSortFunc = std::function<bool(CONST CGameObject*, CONST CGameObject*)>;

class CRenderManager : public CBase
{
private:
    explicit CRenderManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual ~CRenderManager() = default;
private:
    HRESULT Initialize();
private:
    HRESULT Ready_Deferred();
    HRESULT Ready_RasterizerState();
    HRESULT Ready_BlendState();
    HRESULT Ready_DepthStencilState();
public:
    HRESULT Ready_Default();
    HRESULT RenderGruop(const vector<CCameraObjectBase*>& vecRenderCam);
    /* PERCAM : one camera == one render sequence */
    HRESULT RenderGruop(CCameraObjectBase* pRenderCam);
    HRESULT Add_RenderGroup(RENDER_GROUP eRenderGruop , CGameObject* pGameObject );
    HRESULT Clean_RenderGroup();
public:
    /* ---- measurement scaffolding ---- */
    /* SHARED : original. APLAN : shared G-Buffer + partial clear + camera-index discard.
       PERCAM : one camera == one full deferred sequence.                                */
    enum class RENDER_MODE { SHARED, APLAN, PERCAM, END };

    void        Set_RenderMode(RENDER_MODE eMode);
    RENDER_MODE Get_RenderMode() const { return m_eRenderMode; }
    void    Set_PerCameraDeferred(bool isPerCamera);
    bool    Get_PerCameraDeferred() const { return m_isPerCameraDeferred; }
    /* A-plan entry point */
    HRESULT RenderGruop_APlan(const vector<CCameraObjectBase*>& vecRenderCam);
    void    Set_BenchLabel(const string& strLabel) { m_strBenchLabel = strLabel; }
    void    GpuTimer_Begin(uint32 iCamCount);
    void    GpuTimer_End();
#ifdef _DEBUG
    HRESULT Render_DebugPass(const vector<CCameraObjectBase*>& vecRenderCam) { return Render_Debug(vecRenderCam); }
#endif
private:
    HRESULT Ready_GpuTimer();
    void    Flush_GpuAverage();

private:
    HRESULT Render_Priority(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT Render_NonBlend(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT Render_Light(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT Render_Combind(CCameraObjectBase* pRenderCam);
    HRESULT Render_NoneLight(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT Render_Blend(const vector<CCameraObjectBase*>& vecRenderCam);
private:
    void    Set_GPU_BeforeLight();
#ifdef _DEBUG
    HRESULT Render_Debug(const vector<CCameraObjectBase*>& vecRenderCam);
    HRESULT Render_DebugComponent(const vector<CCameraObjectBase*>& vecRenderCam);
#endif 
public:
#ifdef _DEBUG
    HRESULT Add_DebugComponent(CComponent* pComponent);
#endif 

private:
    CGameManager*                                                    m_pGameManager;
    ComPtr<Device>                                                   m_pDevice;
    ComPtr<DeviceContext>                                            m_pDeviceContext;

private:

    unordered_map<wstring,CRenderState*> m_umapRenderState[Cast_Uint(STATE_TYPE::END)];
    Float4x4                                                         m_matWorld;
    Float4x4                                                         m_matProj;
    Float4x4                                                         m_matView;

    Float4x4                                                         m_matGlobalInvProj;

    CShader*                                                         m_pShader;
    CVIBuffer_Rect*                                                  m_pBuffer;
private:
    Engine::RenderGroup                                              m_RenderGroups[Cast_Uint(RENDER_GROUP::END)];
    vector< function<bool(CONST CGameObject*, CONST CGameObject*)>>  m_vecRenderGroupSortFunc;


#ifdef _DEBUG
    vector<CComponent*>                                              m_vecDebugComponent;
#endif 

private:
    /* ---- measurement scaffolding ---- */
    RENDER_MODE                                                      m_eRenderMode;
    bool                                                             m_isPerCameraDeferred;

    static const uint32 GPUTIMER_DEPTH = 3;
    struct GPU_TIMER_FRAME
    {
        ComPtr<Diligent::IQuery> pDuration;
        uint32              iCamCount  = 0;
        int                 iMode      = 0;
        string              strLabel;
        bool                isInFlight = false;
    };
    GPU_TIMER_FRAME                                                  m_tGpuTimer[GPUTIMER_DEPTH];
    uint32                                                           m_iGpuTimerIndex;
    uint32                                                           m_iGpuSampleCount;
    double                                                           m_dGpuTimeSum;
    std::ofstream                                                    m_ofsGpuLog;
    string                                                           m_strBenchLabel;

public:
    static  CRenderManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext );
    virtual void            Free()override;
};

NS_END
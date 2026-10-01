#include "CCameraObjectBase.h"
#include "CGraphicDevice.h"
#include "CCameraComponent.h"
#include "CGameManager.h"
#include "CTransform.h"

USING(Engine)

CCameraObjectBase::CCameraObjectBase(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CGameObject(pDevice, pContext), m_pRTVCom(nullptr),m_eCameraRenderDepth(CAMERA_RENDER_DEPTH::END),
    m_eCameraUpdateDepth(CAMERA_UPDATE_DEPTH::END), m_isDefaultCamera(false)
    , m_eProjectionType(PROJECTION_TYPE::END), m_tInfo{}, m_vRenderAreaXZ{}
{
    for (uint32 i = 0; i < Cast_Uint(UPDATE_PHASE::END); ++i)
    {
        m_isUpdatePahseActive[i] = true;
    }
}




HRESULT CCameraObjectBase::Camera_Initialize(void* pArg)
{
    NULLPTR_RETURN_EFAIL(pArg);

    /* GameObject 초기화 및 Transform 생성할꺼다 */
    if (FAILED(__super::Clone_Initialize(pArg)))
        return E_FAIL;

    /* 카메라 오브젝트 Creaet 할때 필요한 Desc가 있다면 나중에 설정 해보도록 하자 */
    CameraObject_Desc* pDesc = static_cast<CameraObject_Desc*>(pArg);

    /* Depth 세팅 */
    m_eCameraUpdateDepth = pDesc->CameraObjectBase_eCameraUpdateDepth;
    m_eCameraRenderDepth = pDesc->CameraObjectBase_eCameraRenderDepth;

    /* RTV 세팅 */
    m_pRTVCom = pDesc->CameraObjectBase_pRTVCom;    

    /* DSV  세팅*/
    m_pDepthStencilViewCom = pDesc->CameraObjectBase_pDepthStencilViewCom;

    /* bool 값 처리 */
    m_isUpdatePahseActive[Cast_Uint(UPDATE_PHASE::Update)] = pDesc->CameraObjectBase_isUpdateActive;
    m_isUpdatePahseActive[Cast_Uint(UPDATE_PHASE::Render)] = pDesc->CameraObjectBase_isRenderActive;

    /* Cam Info */
    m_tInfo.tViewPortDesc.TopLeftX = pDesc->CameraObjectBase_fScreenStartX;
    m_tInfo.tViewPortDesc.TopLeftY = pDesc->CameraObjectBase_fScreenStartY;
    m_tInfo.tViewPortDesc.Width    = pDesc->CameraObjectBase_fFilmingSizeX;
    m_tInfo.tViewPortDesc.Height   = pDesc->CameraObjectBase_fFilmingSizeY;
    m_tInfo.tViewPortDesc.MaxDepth = 1.f;
    m_tInfo.tViewPortDesc.MinDepth = 0.f;

    m_tInfo.fFar  = pDesc->CameraObjectBase_fFar;
    m_tInfo.fNear = pDesc->CameraObjectBase_fNear;
    m_tInfo.fFov  = pDesc->CameraObjectBase_fFov;

    m_eProjectionType = pDesc->CameraObjectBase_eProjectonType;

    /* Render 영역 결정 */
    m_vRenderAreaXZ = pDesc->CameraObjectBase_RenderAreaXZ;

    if (FAILED(CCameraObjectBase::Ready_Component(pArg)))
        return E_FAIL;


    return S_OK;
}

HRESULT CCameraObjectBase::Ready_Component(void* pArg)
{
    return S_OK;
}


HRESULT CCameraObjectBase::Set_GPU_ViewPort()
{
    CGraphicDevice::Get_Instance()->RS_SetViewports(1,&m_tInfo.tViewPortDesc);
    return S_OK;
}

HRESULT CCameraObjectBase::Set_GPU_RTV()
{
    RenderTargetView* pRTVs = {
        m_pRTVCom.Get()
    };

    CGraphicDevice::Get_Instance()->OM_SetRenderTargets(1, &pRTVs, m_pDepthStencilViewCom.Get());

    return S_OK;
}

HRESULT CCameraObjectBase::Set_GPU_Camera()
{
    CAM_SHADER_INFO tShaderInfo{};
    tShaderInfo.fStartX = m_tInfo.tViewPortDesc.TopLeftX;
    tShaderInfo.fStartY = m_tInfo.tViewPortDesc.TopLeftY;
    tShaderInfo.fWidth  = m_tInfo.tViewPortDesc.Width;
    tShaderInfo.fHeight = m_tInfo.tViewPortDesc.Height;
    tShaderInfo.fFar    = m_tInfo.fFar;
    tShaderInfo.fNear   = m_tInfo.fNear;
    tShaderInfo.fFov    = m_tInfo.fFov;
    /* A-plan only. -1 turns the light-pass index reject off. */
    tShaderInfo.fCamIndex = (m_pGameManager->RenderManager_Get_RenderMode() == 1) ? Cast_float(m_iCamIndex) : -1.f;


    if (FAILED(m_pGameManager->PipeLine_Set_GPU_CameraBuffer(m_strName, tShaderInfo)))
        return E_FAIL;

    return S_OK;
}

HRESULT CCameraObjectBase::Set_ScissorRects()
{
    RECT_DESC tRect;

    tRect.left   = (LONG)m_tInfo.tViewPortDesc.TopLeftX;
    tRect.top    = (LONG)m_tInfo.tViewPortDesc.TopLeftY;
    tRect.right = (LONG)tRect.left + (LONG)m_tInfo.tViewPortDesc.Width;
    tRect.bottom = (LONG)tRect.top + (LONG)m_tInfo.tViewPortDesc.Height;

    CGraphicDevice::Get_Instance()->RS_SetScissorRects(1,&tRect);

    return S_OK;
}

HRESULT CCameraObjectBase::Set_GPU_BeforeRender()
{
    /* 카메라가 가진 Viewport만세팅 화면에 그릴 영역 */
    CGraphicDevice::Get_Instance()->RS_SetViewports(1,&m_tInfo.tViewPortDesc);
    /* RTV 세팅 */
    //CGraphicDevice::Get_Instance()->OM_SetRenderTargets(1, m_pRTVCom.GetAddressOf(), m_pDepthStencilViewCom.Get());

    return S_OK;
}

HRESULT CCameraObjectBase::Set_PipeLineTransform()
{
    if (m_eProjectionType == PROJECTION_TYPE::PERSPECTIVE)
    {
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, m_pTransformCom->Get_WorldMatrixInverse());
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_tInfo.fFov), m_tInfo.tViewPortDesc.Width / m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));

    }
    else // 직교 투영이라면
    {
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, XMMatrixIdentity());
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixOrthographicLH(m_tInfo.tViewPortDesc.Width, m_tInfo.tViewPortDesc.Height , m_tInfo.fNear , m_tInfo.fFar ));
    }

    return S_OK;
}

Float4 CCameraObjectBase::Get_CameraRenderAreaXY_MinMax()
{
    Float4 vPos = m_pTransformCom->Get_State_ToStore(STATE::POSITION);

    return Float4( vPos.x - m_vRenderAreaXZ.x * 0.5f , vPos.x + m_vRenderAreaXZ.x * 0.5f, vPos.z - m_vRenderAreaXZ.y * 0.5f, vPos.z + m_vRenderAreaXZ.y * 0.5f);
}

HRESULT CCameraObjectBase::Update_CameraMatrix()
{
    if (m_eProjectionType == PROJECTION_TYPE::PERSPECTIVE)
    {
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, m_pTransformCom->Get_WorldMatrixInverse());
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_tInfo.fFov), m_tInfo.tViewPortDesc.Width / m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));

    }
    else // 직교 투영이라면
    {
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, XMMatrixIdentity());
        m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixOrthographicLH(m_tInfo.tViewPortDesc.Width, m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));
    }

    return S_OK;
}


#pragma region 하위 객체들에게 정의안해도되게해주기

HRESULT CCameraObjectBase::Add_RenderGroup()
{
    return S_OK;
}

HRESULT CCameraObjectBase::Render()
{
    /* Setting은 알아서 CameraManager가 해주는다는 개념 */

    return S_OK;
}

#pragma endregion

#ifdef _DEBUG
void CCameraObjectBase::ImGuiUpdate()
{
    __super::ImGuiUpdate();
    ImGuiDebugRender();
}

void CCameraObjectBase::ImGuiDebugRender()
{
    if (ImGui::TreeNode("Camera Object Base"))
    {
        if (ImGui::TreeNode("Camera Info"))
        {
            ImGui::DragFloat("Fov", &m_tInfo.fFov, 0.1f, 1.f, 179.f, "%.1f deg");
            ImGui::DragFloat("Near", &m_tInfo.fNear, 0.01f, 0.001f, m_tInfo.fFar - 0.001f, "%.3f");
            ImGui::DragFloat("Far", &m_tInfo.fFar, 0.1f, m_tInfo.fNear + 0.001f, FLT_MAX, "%.3f");

            ImGui::SeparatorText("Viewport");
            ImGui::DragFloat("Top Left X", &m_tInfo.tViewPortDesc.TopLeftX, 1.f);
            ImGui::DragFloat("Top Left Y", &m_tInfo.tViewPortDesc.TopLeftY, 1.f);
            ImGui::DragFloat("Width", &m_tInfo.tViewPortDesc.Width, 1.f, 1.f, FLT_MAX);
            ImGui::DragFloat("Height", &m_tInfo.tViewPortDesc.Height, 1.f, 1.f, FLT_MAX);
            ImGui::SliderFloat("Min Depth", &m_tInfo.tViewPortDesc.MinDepth, 0.f, m_tInfo.tViewPortDesc.MaxDepth);
            ImGui::SliderFloat("Max Depth", &m_tInfo.tViewPortDesc.MaxDepth, m_tInfo.tViewPortDesc.MinDepth, 1.f);

            ImGui::TreePop();
        }

        int iRenderDepth = static_cast<int>(m_eCameraRenderDepth);
        if (ImGui::SliderInt("Render Depth", &iRenderDepth,
            static_cast<int>(CAMERA_RENDER_DEPTH::DEPTH_1),
            static_cast<int>(CAMERA_RENDER_DEPTH::DEPTH_100)))
        {
            m_eCameraRenderDepth = static_cast<CAMERA_RENDER_DEPTH>(iRenderDepth);
        }

        if (ImGui::TreeNode("Render Area Info"))
        {
            ImGui::SliderFloat2("Render Area X", (float*)&m_vRenderAreaXZ, 0, 100);
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}
#endif // _DEBUG



void CCameraObjectBase::Free()
{
    __super::Free();
    /* CCameraObjectBAse::Free() */
}

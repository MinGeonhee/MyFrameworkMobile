#include "CCameraComponent.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CImGuiFuntion.h"
#include "MyImGuiHeader.h"



CCameraComponent::CCameraComponent(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContect)
    : CComponent(pDevice, pDeviceContect) , m_eProjectionType(PROJECTION_TYPE::END)
    , m_fFar(0.f), m_fNear(0.f), m_fFov(0.f), m_tCameraBufferData(), m_tViewPortDesc{}
{
}

CCameraComponent::CCameraComponent(const CCameraComponent& Prototype)
    : CComponent(Prototype), m_tCameraBufferData(Prototype.m_tCameraBufferData)
    , m_eProjectionType(Prototype.m_eProjectionType), m_fFov(Prototype.m_fFov),
    m_fFar(Prototype.m_fFar), m_fNear(Prototype.m_fNear), m_tViewPortDesc(Prototype.m_tViewPortDesc)
{
}

HRESULT CCameraComponent::Proto_Initialize()
{
    return S_OK;
}

HRESULT CCameraComponent::Clone_Initialize(void* pArg)
{
    //NULLPTR_RETURN_EFAIL(pArg);

    //Component_Desc* pCameraDesc = static_cast<Component_Desc*>(pArg);

    //m_eProjectionType = pCameraDesc->pCameraCom_Desc->eProjectonType;

    ///* 카메라 각도 , 깊이 조절 */
    //m_fNear = pCameraDesc->pCameraCom_Desc->fNear;
    //m_fFar = pCameraDesc->pCameraCom_Desc->fFar;
    //m_fFov = pCameraDesc->pCameraCom_Desc->fFov;

    ///* 카메라 시작 지점 */
    //m_tViewPortDesc.TopLeftX = pCameraDesc->pCameraCom_Desc->fScreenStartX;
    //m_tViewPortDesc.TopLeftY = pCameraDesc->pCameraCom_Desc->fScreenStartY;
    ///* 카메라 시작지점으로부터 그릴 크기 */
    //m_tViewPortDesc.Width   =  pCameraDesc->pCameraCom_Desc->fFilmingSizeX;
    //m_tViewPortDesc.Height  =  pCameraDesc->pCameraCom_Desc->fFilmingSizeY;

    ///* Depth 강제 0.1 ~ 1 */
    //m_tViewPortDesc.MaxDepth = 1.f;
    //m_tViewPortDesc.MinDepth = 0.f;


    //Make_MyProjectionMatrix();

    return S_OK;
}


#ifdef _DEBUG

void CCameraComponent::ImGuiUpdate(const wstring& wstrName)
{



    MYIMGUI::Slider_Float Slider; Slider.Set_IsRenderInputFloat(true);
    Slider.SetOnWork([&]() {});
    if (ImGui::TreeNode(" Near / Far / Fov"))
    {

        CImGuiFuntion::ColorText(COLOR_RED, "Camera Near / Far / Fov");
        ImGui::Text(" Near : %.2f     Far : %.2f    Fov : %.2f", m_fNear, m_fFar, m_fFov);


        ImGui::Text(" Near => %.2f", &m_fNear);

        Slider.Set_Slider("Near", &m_fNear, 0.1f, 999.f);
        Slider.Render();

        ImGui::Text(" Far  => %.2f", m_fFar);
        Slider.Set_Slider("Far", &m_fFar, 0.2f, 1000.f);
        Slider.Render();

        ImGui::Text(" Fov  => %.2f", &m_fFov);
        Slider.Set_Slider("Fov", &m_fFov, 0.1f, 1000.f);
        Slider.Render();

        ImGui::TreePop();
    }

    if (ImGui::TreeNode(" Screen "))
    {
        float ScreenCenterX{ m_tViewPortDesc.TopLeftX * m_tViewPortDesc.Width * 0.5f  };
        float ScreenCenterY{ m_tViewPortDesc.TopLeftX * m_tViewPortDesc.Height * 0.5f  };

        CImGuiFuntion::ColorText(COLOR_RED, "Screen Center X , Y");

        ImGui::Text(" Screen Center X : %.2f   Screen Center : %.2f", ScreenCenterX , ScreenCenterY);

        ImGui::Text(" Center X => %.2f", ScreenCenterX);
        Slider.Set_Slider("Screen Center X", &m_tViewPortDesc.TopLeftX, 0.f, 800.f);
        Slider.Render();

        ImGui::Text(" Center Y => %.2f", ScreenCenterX);
        Slider.Set_Slider("Screen Center Y", &m_tViewPortDesc.TopLeftY, 0.f, 600.f);
        Slider.Render();

        ImGui::Text(" Screen Size X : %.2f   Screen Size : %.2f", ScreenCenterX, ScreenCenterY);
        

        ImGui::Text(" Size X  => %.2f", m_fFar);
        Slider.Set_Slider("Size X",&m_tViewPortDesc.Width, 0.1f, 800.f);
        Slider.Render();

        ImGui::Text(" Size Y  => %.2f", &m_fFov);
        Slider.Set_Slider("Size Y", &m_tViewPortDesc.Height, 0.1f, 600.f);
        Slider.Render();

        ImGui::TreePop();
    }

    /* View Space  */
    if (ImGui::TreeNode(" View Space Matrix "))
    {
        Float4x4 matView = m_tCameraBufferData.matView;

        CImGuiFuntion::RenderMatrix(matView);

        ImGui::TreePop();
    }

    /* Projection Matrix */
    if (ImGui::TreeNode(" Projection Matrix "))
    {
        Float4x4 matProj = m_tCameraBufferData.matProj;

        CImGuiFuntion::RenderMatrix(matProj);

        ImGui::TreePop();
    }

}
#endif // _DEBUG


void CCameraComponent::Make_ViewSpaceMatrix_LookAt(const Float4& CameraPos, const Float4& TargetPos)
{
    XMStoreFloat4x4(&m_tCameraBufferData.matView, XMMatrixLookAtLH(XMLoadFloat4(&CameraPos), XMLoadFloat4(&TargetPos), Get_Up_Defualt()));
}

void CCameraComponent::Make_ViewSpaceMatrix_LookAt(const fVector4_1& CameraPos, const fVector4_2& TargetPos)
{
    XMStoreFloat4x4(&m_tCameraBufferData.matView,XMMatrixLookAtLH(CameraPos,TargetPos, Get_Up_Defualt()));
}

void CCameraComponent::Make_ViewSpaceMatrix_LookTo(const Float4& CameraPos, const Float4& DirVec)
{
    XMStoreFloat4x4(&m_tCameraBufferData.matView, XMMatrixLookToLH(XMLoadFloat4(&CameraPos), XMLoadFloat4(&DirVec), Get_Up_Defualt()));
}

void CCameraComponent::Make_ViewSpaceMatrix_LookTo(const fVector4_1& CameraPos, const fVector4_2& DirVec)
{
    XMStoreFloat4x4(&m_tCameraBufferData.matView, XMMatrixLookToLH(CameraPos, DirVec, Get_Up_Defualt()));
}

void CCameraComponent::Make_MyProjectionMatrix()
{
    if (m_eProjectionType == PROJECTION_TYPE::PERSPECTIVE)
        XMStoreFloat4x4(&m_tCameraBufferData.matProj, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fFov), (float)m_tViewPortDesc.Width /  (float)m_tViewPortDesc.Height, m_fNear, m_fFar));
    else if (m_eProjectionType == PROJECTION_TYPE::ORTHOGRAPHIC)
        XMStoreFloat4x4(&m_tCameraBufferData.matProj, XMMatrixOrthographicLH(Cast_float(m_tViewPortDesc.Width), Cast_float(m_tViewPortDesc.Height), m_fNear, m_fFar));
    else
        XMStoreFloat4x4(&m_tCameraBufferData.matProj, XMMatrixIdentity());
}


CAM_BUF_DATA CCameraComponent::Get_Orthographic_CameraConstantBuffer()
{
    CAM_BUF_DATA CB_Cam;

    XMStoreFloat4x4(&CB_Cam.matProj, XMMatrixOrthographicLH(Cast_float(m_tViewPortDesc.Width), Cast_float(m_tViewPortDesc.Height), 0.1f, 1.f));
    /* 직교투영은 둘이 곱한건 그대로 직교투영이다 */
    CB_Cam.matViewProj = CB_Cam.matProj;


    return CB_Cam;
}


HRESULT CCameraComponent::Make_MyViewMatrix_To_Identity()
{
    XMStoreFloat4x4(&m_tCameraBufferData.matView, XMMatrixIdentity());
    return S_OK;
}
HRESULT CCameraComponent::Make_MyProjMatrix_To_Identity()
{
    XMStoreFloat4x4(&m_tCameraBufferData.matProj, XMMatrixIdentity());

    return S_OK;
}

CCameraComponent* CCameraComponent::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContect)
{
    CCameraComponent* pCameraComponent = new CCameraComponent(pDevice, pDeviceContect);

    if (FAILED(pCameraComponent->Proto_Initialize()))
    {
        
        (pCameraComponent);
        MSG_BOX("Camera is failed to Create ");
        return nullptr;
    }

    return pCameraComponent;
}

CComponent* CCameraComponent::Clone(void* pArg)
{
    CCameraComponent* pCameraComponent = new CCameraComponent(*this);

    if (FAILED(pCameraComponent->Clone_Initialize(pArg)))
    {
        Safe_Release(pCameraComponent);
        MSG_BOX("Camera is failed to Create ");
        return nullptr;
    }

    return pCameraComponent;
}

void CCameraComponent::Free()
{
    __super::Free();
}

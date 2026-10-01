#pragma once
#include "CComponent.h"


namespace Engine
{
    class CShader;
    class CCameraObjectInfoWindow;
}


NS_BEGIN(Engine)


class ENGINE_DLL CCameraComponent final :    public CComponent
{
    friend CCameraObjectInfoWindow;

protected:
    explicit CCameraComponent(ComPtr<Device> pDevice , ComPtr<DeviceContext> pDeviceContect);
    explicit CCameraComponent(const CCameraComponent& Prototype);
    virtual ~CCameraComponent() {}
private:
    virtual HRESULT         Proto_Initialize() override;
    virtual HRESULT         Clone_Initialize(void* pArg) override;
#ifdef _DEBUG

    virtual void            ImGuiUpdate(const wstring& wstrName);
#endif
public:

    /* LookAt  내위치 , 타겟포지션 , World UP*/
    void                    Make_ViewSpaceMatrix_LookAt(const Float4& CameraPos, const Float4& TargetPos);
    void                    Make_ViewSpaceMatrix_LookAt(const fVector4_1& CameraPos, const fVector4_2& TargetPos);
    /* Lookto  내위치 , 내 방향벡터 , World UP*/
    void                    Make_ViewSpaceMatrix_LookTo(const Float4& CameraPos, const Float4& DirVec);
    void                    Make_ViewSpaceMatrix_LookTo(const fVector4_1& CameraPos, const fVector4_2& DirVec);
                   
    /*  */
    HRESULT                 Make_MyViewMatrix_To_Identity();
    HRESULT                 Make_MyProjMatrix_To_Identity();

    void                    Make_MyProjectionMatrix();

public:
    /* Get */
    float                   Get_Fov() { return m_fFov; }
    float                   Get_Far() { return m_fFar; }
    float                   Get_Near() { return m_fNear; }
    

    CONST CAM_BUF_DATA*     Get_CameraBufferPonter()                { return &m_tCameraBufferData; }
    CONST CAM_BUF_DATA&     Get_CameraBuffer()                      { return m_tCameraBufferData; }
    CONST Float4x4&         Get_ProjectionMatrix()                  { return m_tCameraBufferData.matProj; }
    CONST Float4x4&         Get_ViewSpaceMatrix()                   { return m_tCameraBufferData.matView; }
    CONST VIEWPORT_DESC&   Get_ViewPort()                          { return m_tViewPortDesc; }


    CAM_BUF_DATA            Get_Orthographic_CameraConstantBuffer();

public:
    /* Set */
    void                    Set_Fov(float fFov)     { m_fFov = fFov; }
    void                    Set_Near(float fNear)   { m_fNear = fNear; }
    void                    Set_Far(float fFar)     { m_fFar = fFar; }

protected:
    PROJECTION_TYPE         m_eProjectionType;

    VIEWPORT_DESC			m_tViewPortDesc;
protected:
                    
    float                   m_fFov;
    float                   m_fNear;
    float                   m_fFar;

protected:
    /* matrix =>  View / Proj / View * Proj */
    CAM_BUF_DATA            m_tCameraBufferData;
public:
    static                  CCameraComponent* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContect);
    // CComponent을(를) 통해 상속됨
    CComponent*             Clone(void* pArg) override;
    virtual void            Free() override;
};

NS_END
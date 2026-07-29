#pragma once
#include "CGameObject.h"
#include "Common/interface/BasicMath.hpp"

class CTransform;

class ENGINE_DLL CCameraObjectBase abstract : public CGameObject
{
public:
    struct CameraObject_Desc : public Engine::CAMERA_OBJECT_DESC {};
protected:
    explicit CCameraObjectBase(Device* pDevice, Context* pContext);
    explicit CCameraObjectBase(const CCameraObjectBase& Prototype);
    ~CCameraObjectBase() override = default;
    virtual HRESULT Camera_Initialize(void* pArg);
    HRESULT Ready_Component(void* pArg) override;
public:
    HRESULT Set_GPU_BeforeRender();
    virtual HRESULT Set_PipeLineTransform();
    HRESULT Update_CameraMatrix();
    HRESULT Add_RenderGroup() override { return S_OK; }
    CTransform* Get_Transform() const { return m_pTransform; }
    const Diligent::float4x4& Get_ViewMatrix() const { return m_ViewMatrix; }
    const Diligent::float4x4& Get_ProjectionMatrix() const { return m_ProjectionMatrix; }
    CAMERA_UPDATE_DEPTH Get_CameraUpdateDepth() const{return m_eCameraUpdateDepth;}
    CAMERA_RENDER_DEPTH Get_CameraRenderDepth() const{return m_eCameraRenderDepth;}
    bool Get_IsDefaultCamera()const{return m_isDefaultCamera;}
    void Resize(float fWidth,float fHeight);
    void Free() override;
protected:
    CTransform* m_pTransform=nullptr;
    Diligent::float4x4 m_ViewMatrix=Diligent::float4x4::Identity();
    Diligent::float4x4 m_ProjectionMatrix=Diligent::float4x4::Identity();
    PROJECTION_TYPE m_eProjectionType=PROJECTION_TYPE::PERSPECTIVE;
    CAMERA_UPDATE_DEPTH m_eCameraUpdateDepth=CAMERA_UPDATE_DEPTH::DEPTH_0;
    CAMERA_RENDER_DEPTH m_eCameraRenderDepth=CAMERA_RENDER_DEPTH::DEPTH_0;
    float m_fScreenStartX=0.f,m_fScreenStartY=0.f,m_fFilmingSizeX=1280.f,m_fFilmingSizeY=720.f;
    float m_fFovRadian=1.04719755f,m_fNear=.1f,m_fFar=1000.f;
    bool m_isDefaultCamera=false;
};

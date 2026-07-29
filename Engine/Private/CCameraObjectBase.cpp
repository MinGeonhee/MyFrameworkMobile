#include "pch.h"
#include "CCameraObjectBase.h"
#include "CTransform.h"
CCameraObjectBase::CCameraObjectBase(Device*d,Context*c):CGameObject{d,c}{}
CCameraObjectBase::CCameraObjectBase(const CCameraObjectBase&p):CGameObject{p},m_eProjectionType{p.m_eProjectionType},m_eCameraUpdateDepth{p.m_eCameraUpdateDepth},m_eCameraRenderDepth{p.m_eCameraRenderDepth},m_fScreenStartX{p.m_fScreenStartX},m_fScreenStartY{p.m_fScreenStartY},m_fFilmingSizeX{p.m_fFilmingSizeX},m_fFilmingSizeY{p.m_fFilmingSizeY},m_fFovRadian{p.m_fFovRadian},m_fNear{p.m_fNear},m_fFar{p.m_fFar},m_isDefaultCamera{p.m_isDefaultCamera}{}
HRESULT CCameraObjectBase::Ready_Component(void* pArg)
{
    auto* d=static_cast<Engine::CAMERA_OBJECT_DESC*>(pArg);
    if(FAILED(Add_Component(Engine::GLOBAL_LEVEL,L"Proto_Component_Transform",L"Com_Transform",d?&d->TransformDesc:nullptr)))return E_FAIL;
    m_pTransform=static_cast<CTransform*>(Find_Component(L"Com_Transform"));return m_pTransform?S_OK:E_FAIL;
}
HRESULT CCameraObjectBase::Camera_Initialize(void* pArg)
{
    if(!pArg||FAILED(CGameObject::Clone_Initialize(pArg)))return E_INVALIDARG;
    auto&d=*static_cast<CameraObject_Desc*>(pArg);m_eProjectionType=d.eProjectionType;m_eCameraUpdateDepth=d.eCameraUpdateDepth;m_eCameraRenderDepth=d.eCameraRenderDepth;
    m_fScreenStartX=d.fScreenStartX;m_fScreenStartY=d.fScreenStartY;m_fFilmingSizeX=d.fFilmingSizeX;m_fFilmingSizeY=d.fFilmingSizeY;m_fFovRadian=d.fFovDegree*0.01745329252f;m_fNear=d.fNear;m_fFar=d.fFar;
    Set_UpdatePhaseActive(UPDATE_PHASE::Update,d.isUpdateActive);Set_UpdatePhaseActive(UPDATE_PHASE::Render,d.isRenderActive);return Update_CameraMatrix();
}
HRESULT CCameraObjectBase::Update_CameraMatrix(){if(!m_pTransform||m_fFilmingSizeX<=0||m_fFilmingSizeY<=0||m_fNear<=0||m_fFar<=m_fNear)return E_FAIL;m_ViewMatrix=m_pTransform->Get_ViewMatrix();m_ProjectionMatrix=m_eProjectionType==PROJECTION_TYPE::PERSPECTIVE?Diligent::float4x4::Projection(m_fFovRadian,m_fFilmingSizeX/m_fFilmingSizeY,m_fNear,m_fFar,false):Diligent::float4x4::Ortho(m_fFilmingSizeX,m_fFilmingSizeY,m_fNear,m_fFar,false);return S_OK;}
HRESULT CCameraObjectBase::Set_PipeLineTransform(){return Update_CameraMatrix();}
HRESULT CCameraObjectBase::Set_GPU_BeforeRender(){if(!m_pDeviceContext)return E_FAIL;Diligent::Viewport v{m_fScreenStartX,m_fScreenStartY,m_fFilmingSizeX,m_fFilmingSizeY,0.f,1.f};m_pDeviceContext->SetViewports(1,&v,static_cast<uint32>(m_fFilmingSizeX),static_cast<uint32>(m_fFilmingSizeY));return S_OK;}
void CCameraObjectBase::Resize(float w,float h){if(w>0&&h>0){m_fFilmingSizeX=w;m_fFilmingSizeY=h;Update_CameraMatrix();}}
void CCameraObjectBase::Free(){m_pTransform=nullptr;CGameObject::Free();}

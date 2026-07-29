#include "pch.h"
#include "CDefaultCameraObject.h"
#include "CGameManager.h"
#include "CTransform.h"
CDefaultCameraObject::CDefaultCameraObject(Device*d,Context*c):CCameraObjectBase{d,c}{m_isDefaultCamera=true;}
CDefaultCameraObject::CDefaultCameraObject(const CDefaultCameraObject&p):CCameraObjectBase{p},m_eCameraType{p.m_eCameraType},m_eMoveType{p.m_eMoveType},m_eUpdateMode{p.m_eUpdateMode}{}
HRESULT CDefaultCameraObject::Ready_Component(void*a){return CCameraObjectBase::Ready_Component(a);}
HRESULT CDefaultCameraObject::Camera_Initialize(void*a){if(FAILED(CCameraObjectBase::Camera_Initialize(a)))return E_FAIL;auto&d=*static_cast<DefaultCameraObject_Desc*>(a);m_eCameraType=d.eCameraType;m_eMoveType=d.eMoveType;m_eUpdateMode=d.eUpdateMode;return S_OK;}
void CDefaultCameraObject::Pre_Update(float dt){CGameObject::Pre_Update(dt);}
void CDefaultCameraObject::FirstPerson_Update(float dt){float r=0,u=0,f=0;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::W))f+=1;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::S))f-=1;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::D))r+=1;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::A))r-=1;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::SPACE))u+=1;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::CONTROL))u-=1;m_pTransform->Move_Local(r,u,f,dt);const float speed=1.5f*dt;float pitch=0,yaw=0;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::UP))pitch-=speed;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::DOWN))pitch+=speed;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::LEFT))yaw-=speed;if(m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::RIGHT))yaw+=speed;m_pTransform->Add_Rotation(pitch,yaw);}
void CDefaultCameraObject::Free_Update(float dt)
{
    FirstPerson_Update(dt);

    // Touch 0: 화면 왼쪽의 가상 이동 스틱. UI 이미지 없이 시작점을 중심으로 사용한다.
    const auto& MoveTouch=m_pGameManager->InputManager_GetTouchState(0);
    if(MoveTouch.isPressed)
    {
        constexpr float Radius=80.f;
        const float right=std::clamp((MoveTouch.fX-MoveTouch.fStartX)/Radius,-1.f,1.f);
        const float forward=std::clamp((MoveTouch.fStartY-MoveTouch.fY)/Radius,-1.f,1.f);
        m_pTransform->Move_Local(right,0.f,forward,dt);
    }

    // Touch 1: 화면 오른쪽 드래그로 자유 카메라의 Pitch/Yaw를 회전한다.
    const auto& LookTouch=m_pGameManager->InputManager_GetTouchState(1);
    if(LookTouch.isPressed)
    {
        constexpr float Sensitivity=0.004f;
        m_pTransform->Add_Rotation(LookTouch.fDeltaY*Sensitivity,LookTouch.fDeltaX*Sensitivity);
    }
}
void CDefaultCameraObject::Update(float dt){CGameObject::Update(dt);if(!m_pTransform)return;if(m_eCameraType==CAMERA_TYPE::FIRSTPERSON)FirstPerson_Update(dt);else if(m_eCameraType==CAMERA_TYPE::FREE)Free_Update(dt);Update_CameraMatrix();}
void CDefaultCameraObject::Late_Update(float dt){CGameObject::Late_Update(dt);}
CDefaultCameraObject* CDefaultCameraObject::Create(Device*d,Context*c){auto*p=new CDefaultCameraObject{d,c};if(FAILED(p->Proto_Initialize())){p->Release();return nullptr;}return p;}
CGameObject* CDefaultCameraObject::Clone(void*a){auto*p=new CDefaultCameraObject{*this};if(FAILED(p->Camera_Initialize(a))){p->Release();return nullptr;}return p;}
void CDefaultCameraObject::Free(){CCameraObjectBase::Free();}

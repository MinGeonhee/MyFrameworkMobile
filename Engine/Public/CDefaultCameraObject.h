#pragma once
#include "CCameraObjectBase.h"
class ENGINE_DLL CDefaultCameraObject final:public CCameraObjectBase
{
public:struct DefaultCameraObject_Desc:public CameraObject_Desc{MOVE_TYPE eMoveType=MOVE_TYPE::NORMAL;CAMERA_TYPE eCameraType=CAMERA_TYPE::FIRSTPERSON;UPDATE_MODE eUpdateMode=UPDATE_MODE::DYNAMIC;};
private:explicit CDefaultCameraObject(Device*d,Context*c);explicit CDefaultCameraObject(const CDefaultCameraObject&p);~CDefaultCameraObject()override=default;
    HRESULT Camera_Initialize(void* pArg)override;HRESULT Ready_Component(void* pArg)override;
public:
    void Pre_Update(float dt)override;void Update(float dt)override;void Late_Update(float dt)override;
    void Set_CameraType(CAMERA_TYPE t){m_eCameraType=t;}CAMERA_TYPE Get_CameraType()const{return m_eCameraType;}MOVE_TYPE Get_MoveType()const{return m_eMoveType;}UPDATE_MODE Get_UpdateMode()const{return m_eUpdateMode;}
    static CDefaultCameraObject* Create(Device*d,Context*c);CGameObject* Clone(void*pArg)override;void Add_ColliderGroup()override{}void Free()override;
private:void FirstPerson_Update(float dt);void Free_Update(float dt);
    CAMERA_TYPE m_eCameraType=CAMERA_TYPE::FIRSTPERSON;MOVE_TYPE m_eMoveType=MOVE_TYPE::NORMAL;UPDATE_MODE m_eUpdateMode=UPDATE_MODE::DYNAMIC;
};

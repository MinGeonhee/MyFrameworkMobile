#include "CDefaultCameraObject.h"
#include "CGameManager.h"
#include "CCameraComponent.h"
#include "CTransform.h"
#include "CConstantBuffer.h"
#include "CCameraObjectBase.h"
#include "CUICameraObject.h"
#include "CImGuiFuntion.h"
#include "MyImGuiHeader.h"


CDefaultCameraObject::CDefaultCameraObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CCameraObjectBase(pDevice, pContext), m_eCameraType(CAMERA_TYPE::END),m_tDefualtLookVec{} 
    , m_vecRenderGroupID{}, m_eMoveType(MOVE_TYPE::NORMAL), m_isFixedMouse{ false }, m_tOffsetFromTarget{}
    , m_tDefaultLook{}
{
    m_isDefaultCamera = true;
}

HRESULT CDefaultCameraObject::Camera_Initialize(void* pArg)
{
    NULLPTR_RETURN_EFAIL(pArg);

    /* GameObject 초기화 및 Transform 생성할꺼다 */
    if (FAILED(__super::Camera_Initialize(pArg)))
        return E_FAIL;

    /* 카메라 오브젝트 Creaet 할때 필요한 Desc가 있다면 나중에 설정 해보도록 하자 */
    DefaultCameraObject_Dest* pDefaultCameraObjectDesc = static_cast<DefaultCameraObject_Dest*>(pArg);

    m_eCameraType   =   pDefaultCameraObjectDesc->eCameraType;
    m_eMoveType     =   pDefaultCameraObjectDesc->eMoveType;


    m_tOffsetFromTarget =   pDefaultCameraObjectDesc->tOffsetFromTarget;
    m_pTargetObject     =   pDefaultCameraObjectDesc->pActor;

    /* Target이 없는 기본 카메라는 자유 카메라로 사용한다. */
    if (m_pTargetObject == nullptr)
        m_eCameraType = CAMERA_TYPE::FREE;

    m_vRenderAreaXZ = { 48.f , 48.f };
    
    Set_CameraType(m_eCameraType);

    /* 내가 필요한 컴포넌트 추가 */
    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;


    return S_OK;
}

HRESULT CDefaultCameraObject::Ready_Component(void* pArg)
{
    return S_OK;
}


#ifdef _DEBUG

void CDefaultCameraObject::ImGuiUpdate()
{
    __super::ImGuiUpdate();

    MYIMGUI::Slider_Float Slider; Slider.Set_IsRenderInputFloat(true);
    Slider.SetOnWork([&]() {});

    if (ImGui::TreeNode("Camre Type & Move Type"))
    {
        MYIMGUI::EnumRadioButton<CAMERA_TYPE> CameraType_Radio(m_eCameraType);
        const CAMERA_TYPE eSelectedCameraType = CameraType_Radio.Render_Radio();
        if (m_eCameraType != eSelectedCameraType)
        {
            m_eCameraType = eSelectedCameraType;
            Set_CameraType(m_eCameraType);
        }

        MYIMGUI::EnumRadioButton<MOVE_TYPE> MoveType_Radio(m_eMoveType);
        m_eMoveType = MoveType_Radio.Render_Radio();
        ImGui::TreePop();
    }

    if (m_eCameraType == CAMERA_TYPE::FREE && ImGui::TreeNode("Free Camera Control"))
    {
        Float4 vPosition = m_pTransformCom->Get_State_ToStore(STATE::POSITION);
        if (ImGui::DragFloat3("Position", &vPosition.x, 0.1f))
            m_pTransformCom->Set_State(STATE::POSITION, vPosition);

        const Float3 vAccEuler = m_pTransformCom->Get_AccEuler();
        Float3 vRotationDegree{
            XMConvertToDegrees(vAccEuler.x),
            XMConvertToDegrees(vAccEuler.y),
            XMConvertToDegrees(vAccEuler.z)
        };
        if (ImGui::DragFloat3("Rotation", &vRotationDegree.x, 0.5f, -360.f, 360.f, "%.1f deg"))
        {
            m_pTransformCom->Rotation_Default(
                XMConvertToRadians(vRotationDegree.x),
                XMConvertToRadians(vRotationDegree.y),
                XMConvertToRadians(vRotationDegree.z));
        }

		static float fRotationStepDegree = 5.f;
		ImGui::DragFloat("Rotation Step", &fRotationStepDegree, 0.5f, 0.5f, 90.f, "%.1f deg");
		const auto RotateCamera = [this](float fPitchDegree, float fYawDegree, float fRollDegree)
		{
			const Float3 vCurrentEuler = m_pTransformCom->Get_AccEuler();
			m_pTransformCom->Rotation_Default(
				vCurrentEuler.x + XMConvertToRadians(fPitchDegree),
				vCurrentEuler.y + XMConvertToRadians(fYawDegree),
				vCurrentEuler.z + XMConvertToRadians(fRollDegree));
		};

		if (ImGui::Button("Pitch Up")) RotateCamera(-fRotationStepDegree, 0.f, 0.f);
		ImGui::SameLine();
		if (ImGui::Button("Pitch Down")) RotateCamera(fRotationStepDegree, 0.f, 0.f);
		if (ImGui::Button("Yaw Left")) RotateCamera(0.f, -fRotationStepDegree, 0.f);
		ImGui::SameLine();
		if (ImGui::Button("Yaw Right")) RotateCamera(0.f, fRotationStepDegree, 0.f);
		if (ImGui::Button("Roll Left")) RotateCamera(0.f, 0.f, -fRotationStepDegree);
		ImGui::SameLine();
		if (ImGui::Button("Roll Right")) RotateCamera(0.f, 0.f, fRotationStepDegree);

        float fMoveSpeed = m_pTransformCom->Get_SpeedPerSec();
        if (ImGui::DragFloat("Move Speed", &fMoveSpeed, 0.1f, 0.1f, 100.f))
            m_pTransformCom->Set_SpeedPerSec(fMoveSpeed);

        float fRotationSpeedDegree = XMConvertToDegrees(m_pTransformCom->Get_RotationPerSec());
        if (ImGui::DragFloat("Rotation Speed", &fRotationSpeedDegree, 1.f, 1.f, 360.f, "%.1f deg/s"))
            m_pTransformCom->Set_RotationPerSec(XMConvertToRadians(fRotationSpeedDegree));

        ImGui::SeparatorText("Zoom");
        ImGui::SliderFloat("FOV", &m_tInfo.fFov, 1.f, 179.f, "%.1f deg");
        if (ImGui::Button("Zoom In"))
        {
            m_tInfo.fFov -= 5.f;
            if (m_tInfo.fFov < 1.f)
                m_tInfo.fFov = 1.f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out"))
        {
            m_tInfo.fFov += 5.f;
            if (m_tInfo.fFov > 179.f)
                m_tInfo.fFov = 179.f;
        }

        static float fDollyStep = 1.f;
        ImGui::DragFloat("Move Step", &fDollyStep, 0.1f, 0.1f, 100.f);

        const auto MoveCamera = [this](STATE eDirection, float fSign)
        {
            const Vector4 vNextPosition = m_pTransformCom->Get_State_ToCal(STATE::POSITION)
                + XMVector3Normalize(m_pTransformCom->Get_State_ToCal(eDirection)) * fDollyStep * fSign;
            m_pTransformCom->Set_State(STATE::POSITION, vNextPosition);
        };

        if (ImGui::Button("Forward")) MoveCamera(STATE::LOOK, 1.f);
        ImGui::SameLine();
        if (ImGui::Button("Back")) MoveCamera(STATE::LOOK, -1.f);
        ImGui::SameLine();
        if (ImGui::Button("Left")) MoveCamera(STATE::RIGHT, -1.f);
        ImGui::SameLine();
        if (ImGui::Button("Right")) MoveCamera(STATE::RIGHT, 1.f);

        if (ImGui::Button("Up")) MoveCamera(STATE::UP, 1.f);
        ImGui::SameLine();
        if (ImGui::Button("Down")) MoveCamera(STATE::UP, -1.f);

        ImGui::TextDisabled("Position input or buttons control this Free Camera.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode(" Camera Offset From Target "))
    {
        Slider.Set_Slider("Offset  X", &m_tOffsetFromTarget.x, -100.f, 100.f);
        ImGui::Text("Offset X => %.2f", m_tOffsetFromTarget.x);
        Slider.Render();

        Slider.Set_Slider("Offset  Y", &m_tOffsetFromTarget.y, -100.f, 100.f);
        ImGui::Text("Offset Y => %.2f", m_tOffsetFromTarget.y);
        Slider.Render();

        Slider.Set_Slider("Offset  z", &m_tOffsetFromTarget.z, -100.f, 100.f);
        ImGui::Text("Offset Z => %.2f", m_tOffsetFromTarget.z);
        Slider.Render();

        ImGui::TreePop();
    }

    for (auto Component : m_mapComponent)
    {

    }
}
#endif // _DEBUG


#pragma region  타입별 세팅 함수

void    CDefaultCameraObject::Set_CameraType(CAMERA_TYPE eType)
{
    HRESULT hr{ E_FAIL };

    switch (eType)
    {
    case Engine::CAMERA_TYPE::FIRSTPERSON:
        hr = Setting_FirstPersonType();
        break;
    case Engine::CAMERA_TYPE::THIRDPERSON:
        hr = Setting_ThirdPersonType();
        break;
    case Engine::CAMERA_TYPE::FREE:
        hr = Setting_FreeType();
        break;
    case Engine::CAMERA_TYPE::FIXED:
        hr = Setting_FixedType();
        break;

    default:
        break;
    }
}

HRESULT CDefaultCameraObject::Set_PipeLineTransform()
{
    switch (m_eCameraType)
    {
    case Engine::CAMERA_TYPE::FIRSTPERSON:
        break;
    case Engine::CAMERA_TYPE::THIRDPERSON:
        if (m_eProjectionType == PROJECTION_TYPE::PERSPECTIVE)
        {
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, XMMatrixLookToLH(m_pTransformCom->Get_State_ToCal(STATE::POSITION),XMLoadFloat3(&m_tDefualtLookVec),Get_Up_Defualt()));
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_tInfo.fFov), m_tInfo.tViewPortDesc.Width / m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));

        }
        else 
        {
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, XMMatrixIdentity());
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixOrthographicLH(m_tInfo.tViewPortDesc.Width, m_tInfo.tViewPortDesc.Height,m_tInfo.fNear, m_tInfo.fFar));
        }
        break;
    case Engine::CAMERA_TYPE::FREE:
        if (m_eProjectionType == PROJECTION_TYPE::PERSPECTIVE)
        {
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, m_pTransformCom->Get_WorldMatrixInverse());
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_tInfo.fFov), m_tInfo.tViewPortDesc.Width / m_tInfo.tViewPortDesc.Height, m_tInfo.fNear , m_tInfo.fFar));

        }
        else // 직교 투영이라면
        {
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::VIEW, XMMatrixIdentity());
            m_pGameManager->PipeLine_Set_PipeLineMatrix(m_strName, D3DTS::PROJ, XMMatrixOrthographicLH(m_tInfo.tViewPortDesc.Width, m_tInfo.tViewPortDesc.Height, m_tInfo.fNear, m_tInfo.fFar));
        }
        break;
    case Engine::CAMERA_TYPE::FIXED:
        break;
    case Engine::CAMERA_TYPE::END:
        break;
    default:
        break;
    }

    return S_OK;
}

HRESULT CDefaultCameraObject::Setting_FirstPersonType()
{
    return S_OK;
}
HRESULT CDefaultCameraObject::Setting_ThirdPersonType()
{
    Set_DefaultLookVec();

    return S_OK;
}
HRESULT CDefaultCameraObject::Setting_FreeType()
{
    return S_OK;
}
HRESULT CDefaultCameraObject::Setting_FixedType()
{
    return S_OK;
}
#pragma endregion

#pragma region Update

void CDefaultCameraObject::Pre_Update(float DT)
{

    /* CMainCameraObject::Pre_Update */

}
void CDefaultCameraObject::Update(float DT)
{

    /* CMainCameraObject::Update */

}

void CDefaultCameraObject::Late_Update(float DT)
{
    /* CMainCameraObject::Late_Update */


    switch (m_eCameraType)
    {
    case Engine::CAMERA_TYPE::FIRSTPERSON:
        FirstPerson_Update(DT);
        break;
    case Engine::CAMERA_TYPE::THIRDPERSON:
        ThirdPerson_Update(DT);
        break;
    case Engine::CAMERA_TYPE::FREE:
        Free_Update(DT);
        break;
    case Engine::CAMERA_TYPE::FIXED:
        Fixed_Update(DT);
    }


    /* 타입별 카메라 업데이트가 다 끝난뒤 파이프라인에 세팅해준다 */
    if (FAILED(Set_PipeLineTransform()))
        return;

    return;
}

#pragma endregion

#pragma region  Add_Render & Render

HRESULT CDefaultCameraObject::Add_RenderGroup()
{

    return S_OK;
}


void CDefaultCameraObject::Free_KeyUpdaate(float DT)
{
    if (m_eCameraType != CAMERA_TYPE::FREE)
        return;

    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::W))
        m_pTransformCom->Go_Straight(DT);
    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::S))
        m_pTransformCom->Go_Back(DT);
    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::A))
        m_pTransformCom->Go_Left(DT);
    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::D))
        m_pTransformCom->Go_Right(DT);
    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::Q))
        m_pTransformCom->Go_Down(DT);
    if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::E))
        m_pTransformCom->Go_Up(DT);
    //if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::F1))
    //    m_isFixedMouse = !m_isFixedMouse;
}

#pragma endregion

#pragma region 카메라 타입별 업데이트
void CDefaultCameraObject::FirstPerson_Update(float DT)
{
    /*  1인칭 뷰  */
}
void CDefaultCameraObject::ThirdPerson_Update(float DT)
{
    /* 3 인칭 뷰  */

    /* 타겟 없을떄 Return*/
    NULLPTR_RETURN_NULL(m_pTargetObject);

    /* 타겟이 있을 때*/
    if (m_eMoveType == MOVE_TYPE::SMOOTH)
        m_pTransformCom->ChaseSmoothTarget_Offset(m_pTargetObject->GetTransform()->Get_State_ToCal(STATE::POSITION), m_tOffsetFromTarget, DT * 4, 0.1f);
    else if (m_eMoveType == MOVE_TYPE::NORMAL)
        m_pTransformCom->Set_State(STATE::POSITION, m_pTargetObject->GetTransform()->Get_State_ToCal(STATE::POSITION) + XMLoadFloat3(&m_tOffsetFromTarget));
}
void CDefaultCameraObject::Free_Update(float DT)
{
	Free_KeyUpdaate(DT);

	const long iMouseDeltaX = m_pGameManager->InputManager_Get_DIMouseMove(DIMS_X);
	const long iMouseDeltaY = m_pGameManager->InputManager_Get_DIMouseMove(DIMS_Y);
	const long iMouseWheel  = m_pGameManager->InputManager_Get_DIMouseMove(DIMS_Z);

	/* ImGui 조작과 충돌하지 않도록 우클릭 드래그 중에만 회전한다. */
	if (m_pGameManager->InputManager_GetButtonPress(KEY_TYPE::RBUTTON))
	{
		constexpr float fMouseSensitivity = 0.0025f;
		m_pTransformCom->Rotation_AccRadian(
			static_cast<float>(iMouseDeltaY) * fMouseSensitivity,
			static_cast<float>(iMouseDeltaX) * fMouseSensitivity,
			0.f);
	}

	if (iMouseWheel != 0)
	{
		const float fWheelDirection = iMouseWheel > 0 ? 1.f : -1.f;
		const Vector4 vNextPosition = m_pTransformCom->Get_State_ToCal(STATE::POSITION)
			+ XMVector3Normalize(m_pTransformCom->Get_State_ToCal(STATE::LOOK)) * fWheelDirection * 2.f;
		m_pTransformCom->Set_State(STATE::POSITION, vNextPosition);
	}
}
void CDefaultCameraObject::Fixed_Update(float DT)
{

}
#pragma endregion

void CDefaultCameraObject::Set_DefaultLookVec()
{
    if (m_eCameraType != CAMERA_TYPE::THIRDPERSON) return;
    if (m_pTargetObject == nullptr) return;


    m_pTransformCom->Set_State(STATE::POSITION, m_pTargetObject->GetTransform()->Get_State_ToCal(STATE::POSITION) + XMLoadFloat3(&m_tOffsetFromTarget));

    /*  처음에 지정된 플레이어를 향해 설정된 방향 벡터는 그대로일것이다 0 5 5 에서 플레이어를 바라보던 그 방향벡터는 그대로. */
    Vector4 MyPos = m_pTransformCom->Get_State_ToCal(STATE::POSITION);
    Vector4 TargetPos = m_pTargetObject->GetTransform()->Get_State_ToCal(STATE::POSITION);

    XMStoreFloat3(&m_tDefualtLookVec, XMVector3Normalize(TargetPos - MyPos));
}



CDefaultCameraObject* CDefaultCameraObject::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, void* pArg)
{
    CDefaultCameraObject* pMainCameraObject = new CDefaultCameraObject(pDevice, pContext);

    if (FAILED(pMainCameraObject->Camera_Initialize(pArg)))
    {
        Safe_Release(pMainCameraObject);
        MSG_BOX("CameraObject is failed to Create");
        return nullptr;
    }


    return pMainCameraObject;
}

void CDefaultCameraObject::Free()
{
    __super::Free();

    /* CCameraObject::Free() */
}

void CDefaultCameraObject::Add_ColliderGroup()
{
}

CDefaultCameraObject* CDefaultCameraObject::Clone(void* pArg)
{
    return nullptr;
}

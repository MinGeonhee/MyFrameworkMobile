#include "CTransform.h"
#include "MyImGuiHeader.h"
#include "EnumUtils.h"
#include "CGameObject.h"
#include "CImGuiFuntion.h"
#include "CNavigation.h"
#include "CCameraObjectBase.h"

CTransform::CTransform(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CComponent(pDevice, pContext), m_fRotationPerSec{ 0 }, m_fSpeedPerSec{ 0 }, m_tWorldMatrix{} , m_fAccTime(0) , m_tAccEuler{}
{
    XMStoreFloat4x4(&m_tWorldMatrix,XMMatrixIdentity());
}

CTransform::CTransform(const CTransform& Prototype)
    : CComponent(Prototype) , m_fRotationPerSec{ Prototype.m_fRotationPerSec }, m_fSpeedPerSec{ Prototype.m_fSpeedPerSec }, m_tWorldMatrix{ Prototype.m_tWorldMatrix}
    , m_fAccTime(0), m_tAccEuler{ Prototype .m_tAccEuler}
{
}

CTransform::~CTransform()
{
}

HRESULT CTransform::Proto_Initialize()
{
    return S_OK;
}

HRESULT CTransform::Clone_Initialize(void* pArg)
{
    NULLPTR_RETURN_EFAIL(pArg);

    if (FAILED(__super::Clone_Initialize(pArg)))
        return E_FAIL;

    TRANSFORMCOM_DESC* pTransformComDesc = static_cast<TRANSFORMCOM_DESC*>(pArg);

    /* Transform Setting */
    m_fRotationPerSec = pTransformComDesc->fRotationPerSec;
    m_fSpeedPerSec = pTransformComDesc->fSpeedPerSec;




    Set_Scale(pTransformComDesc->tScale);
    Set_State(STATE::POSITION , XMVectorSetW(XMLoadFloat3(&pTransformComDesc->tTransform),1.f));
    Rotation_Default(pTransformComDesc->tRotation);


    return S_OK;
}

#pragma region  Get State
 
Vector4 CTransform::Get_State_ToCal(STATE eState) CONST
{
    /*  저장용으로 들고있던 World Matrix 를 계산용으로 return 해준다 */
    return XMLoadFloat4x4(&m_tWorldMatrix).r[Cast_Uint(eState)];
}

Float4 CTransform::Get_State_ToStore(STATE eState) CONST
{
    Float4 newFloat = {};
    memcpy( &newFloat , &m_tWorldMatrix.m[Cast_Uint(eState)], sizeof(Float4));
    return  newFloat;
}

void CTransform::Set_State(STATE eState, const Float4& vFloat4)
{
    memcpy(&m_tWorldMatrix.m[Cast_Uint(eState)], &vFloat4 , sizeof(Float4));
}
void CTransform::Set_State(STATE eState, const fVector4_1& vVector4)
{
    XMStoreFloat4( reinterpret_cast<XMFLOAT4*>(&m_tWorldMatrix.m[Cast_Uint(eState)]) ,  vVector4 );
}
void CTransform::Set_Position(const fVector4_1& vPos, CNavigation* pNavigation)
{
    if (pNavigation == nullptr)
    {
        XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m_tWorldMatrix.m[Cast_Uint(Engine::STATE::POSITION)]), vPos);
        return;
    }
    
    Vector4 vMyPos = Get_State_ToCal(STATE::POSITION);
}

void CTransform::Set_WorldMatrix(const Float4& vRight, const Float4& vUp, const Float4& vLook, const Float4& vPos)
{
    Set_State(STATE::RIGHT, vRight);
    Set_State(STATE::UP, vUp);
    Set_State(STATE::LOOK, vLook);
    Set_State(STATE::POSITION, vPos);
}

void CTransform::Set_WorldMatrix(const fVector4_1& vRight, const fVector4_2& vUp, const fVector4_3& vLook, const GVector4_4& vPos)
{
    Set_State(STATE::RIGHT, vRight);
    Set_State(STATE::UP, vUp);
    Set_State(STATE::LOOK, vLook);
    Set_State(STATE::POSITION, vPos);
}

void CTransform::Set_WorldMatrix(const XMatrix_1& matWorld)
{
    XMStoreFloat4x4(&m_tWorldMatrix,matWorld);
}

void CTransform::Get_SRT(Float4* vScale, Float4* vRot, Float4* vTrans)
{
    Vector4 S{}, R{}, T{};


    XMMatrixDecompose(&S,&R,&T,XMLoadFloat4x4(&m_tWorldMatrix));

    XMStoreFloat4(vScale,S);
    XMStoreFloat4(vRot,R);
    XMStoreFloat4(vTrans,T);
}

float CTransform::Get_Length(const Float4& vPos) CONST
{
    return XMVectorGetX(XMVector4Length(XMLoadFloat4(&vPos) - Get_State_ToCal(STATE::POSITION)));
}

float CTransform::Get_Length(const fVector4_1& vPos) CONST
{
    return XMVectorGetX(XMVector4Length(vPos - Get_State_ToCal(STATE::POSITION)));
}

float CTransform::Get_LengthSq(const Float4& vPos) const
{
    return XMVectorGetX(XMVector4LengthSq(XMLoadFloat4(&vPos) - Get_State_ToCal(STATE::POSITION)));
}

float CTransform::Get_LengthSq(const fVector4_1& vPos) const
{
    return XMVectorGetX(XMVector4LengthSq(vPos - Get_State_ToCal(STATE::POSITION)));
}

bool CTransform::Get_IsInCamArea(const Float4& vRenderCamAreaXZ)
{

    /* X 영역의 Min x Max y */
    /* Min 보다 작거나 Max 보다 크다면 거르기 */
    if (vRenderCamAreaXZ.x > m_tWorldMatrix._41) return false;
    if (vRenderCamAreaXZ.y < m_tWorldMatrix._41) return false;

    if (vRenderCamAreaXZ.z > m_tWorldMatrix._43) return false;
    if (vRenderCamAreaXZ.w < m_tWorldMatrix._43) return false;

    return true;
}


#pragma endregion



#pragma region Scale


Vector4 CTransform::GetScale_ToCal() CONST
{
    /* 각 Right , Up , Look의 길이 = Scale 을 추출해서 계산용 Vector4 로 return.  */
    return XMVectorSet(
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::RIGHT))),
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::LOOK))),
        0.f
    );
}

Float3 CTransform::GetScale_ToStore() CONST
{
    /* 저장용 Float3로 return */
    return XMFLOAT3(
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::RIGHT))),
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::LOOK)))
    );
}

float CTransform::GetScaleX() CONST
{
    return XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::RIGHT)));
}
float CTransform::GetScaleY() CONST
{
    return XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::UP)));
}
float CTransform::GetScaleZ() CONST
{
    return XMVectorGetX(XMVector3Length(Get_State_ToCal(STATE::LOOK)));
}


void CTransform::Set_Scale(const fVector4_1& vScale, CALCULATE_TYPE eCalType)
{
    Vector4 vecMyScale = GetScale_ToCal();

    Vector4 Cal_Scale{};
    if (eCalType == CALCULATE_TYPE::DEFAULT)    Cal_Scale = vScale;
    if (eCalType == CALCULATE_TYPE::ADD)        Cal_Scale = vecMyScale + vScale;
    if (eCalType == CALCULATE_TYPE::SUB)        Cal_Scale = vecMyScale - vScale;
    if (eCalType == CALCULATE_TYPE::MUL)        Cal_Scale = vecMyScale * vScale;
    if (eCalType == CALCULATE_TYPE::DIV)        Cal_Scale = vecMyScale / vScale;
   
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State_ToCal(STATE::RIGHT)) * XMVectorGetX(Cal_Scale));
    Set_State(STATE::UP,    XMVector3Normalize(Get_State_ToCal(STATE::UP))    * XMVectorGetY(Cal_Scale));
    Set_State(STATE::LOOK,  XMVector3Normalize(Get_State_ToCal(STATE::LOOK))  * XMVectorGetZ(Cal_Scale));
}
void CTransform::Set_Scale(const Float3& vScale, CALCULATE_TYPE eCalType)
{
    Vector4 vecCalScale = XMLoadFloat3(&vScale);
    Vector4 vecMyScale = GetScale_ToCal();

    Vector4 Cal_Scale{};
    if (eCalType == CALCULATE_TYPE::DEFAULT)    Cal_Scale = vecCalScale;
    if (eCalType == CALCULATE_TYPE::ADD)        Cal_Scale = vecMyScale + vecCalScale;
    if (eCalType == CALCULATE_TYPE::SUB)        Cal_Scale = vecMyScale - vecCalScale;
    if (eCalType == CALCULATE_TYPE::MUL)        Cal_Scale = vecMyScale * vecCalScale;
    if (eCalType == CALCULATE_TYPE::DIV)        Cal_Scale = vecMyScale / vecCalScale;


    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State_ToCal(STATE::RIGHT)) * XMVectorGetX(Cal_Scale));
    Set_State(STATE::UP,    XMVector3Normalize(Get_State_ToCal(STATE::UP))    * XMVectorGetY(Cal_Scale));
    Set_State(STATE::LOOK,  XMVector3Normalize(Get_State_ToCal(STATE::LOOK))  * XMVectorGetZ(Cal_Scale));
}
void CTransform::Set_Scale(float fX, float fY, float fZ, CALCULATE_TYPE eCalType)
{
    Vector4 vecCalScale = { fX, fY , fZ , 0.f };
    Vector4 vecMyScale = GetScale_ToCal();

    Vector4 Cal_Scale{};
    if (eCalType == CALCULATE_TYPE::DEFAULT)    Cal_Scale = vecCalScale;
    if (eCalType == CALCULATE_TYPE::ADD)        Cal_Scale = vecMyScale + vecCalScale;
    if (eCalType == CALCULATE_TYPE::SUB)        Cal_Scale = vecMyScale - vecCalScale;
    if (eCalType == CALCULATE_TYPE::MUL)        Cal_Scale = vecMyScale * vecCalScale;
    if (eCalType == CALCULATE_TYPE::DIV)        Cal_Scale = vecMyScale / vecCalScale;

    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State_ToCal(STATE::RIGHT)) * XMVectorGetX(Cal_Scale));
    Set_State(STATE::UP,    XMVector3Normalize(Get_State_ToCal(STATE::UP))    * XMVectorGetY(Cal_Scale));
    Set_State(STATE::LOOK,  XMVector3Normalize(Get_State_ToCal(STATE::LOOK))  * XMVectorGetZ(Cal_Scale));

}
#pragma endregion


bool CTransform::Check_InDistanceRange(const fVector4_1& vTargetPos, float fRange)
{
    XMVECTOR vMyPos = this->Get_State_ToCal(Engine::STATE::POSITION);

    return fRange > fabsf(XMVectorGetX(XMVector3LengthSq(vMyPos - vTargetPos)));
}

#pragma region  Move

void CTransform::Go_Straight(float DT, CNavigation* pNavigation)
{
    Vector4 vResultPos{};
    if (pNavigation) vResultPos = pNavigation->Is_MoveSliding(Get_State_ToCal(STATE::LOOK), Get_State_ToCal(STATE::POSITION), m_fSpeedPerSec * DT);
    else vResultPos = GetPosition_ToCal() + XMVector3Normalize(Get_State_ToCal(STATE::LOOK)) * m_fSpeedPerSec * DT;

    Set_State(STATE::POSITION,vResultPos);
    return;
}

void CTransform::Go_Back(float DT)
{
    Set_State(STATE::POSITION, Get_State_ToCal(STATE::POSITION) - XMVector3Normalize(Get_State_ToCal(STATE::LOOK)) * m_fSpeedPerSec * DT);
}

void CTransform::Go_Left(float DT)
{
    Set_State(STATE::POSITION, Get_State_ToCal(STATE::POSITION) - XMVector3Normalize(Get_State_ToCal(STATE::RIGHT)) * m_fSpeedPerSec * DT);
}

void CTransform::Go_Right(float DT)
{
    Set_State(STATE::POSITION, Get_State_ToCal(STATE::POSITION) + XMVector3Normalize(Get_State_ToCal(STATE::RIGHT)) * m_fSpeedPerSec * DT);
}

void CTransform::Go_Up(float DT)
{
    Set_State(STATE::POSITION, Get_State_ToCal(STATE::POSITION) + XMVector3Normalize(Get_State_ToCal(STATE::UP)) * m_fSpeedPerSec * DT);
}

void CTransform::Go_Down(float DT)
{
    Set_State(STATE::POSITION, Get_State_ToCal(STATE::POSITION) - XMVector3Normalize(Get_State_ToCal(STATE::UP)) * m_fSpeedPerSec * DT);
}

void CTransform::Move(const Float4& vFloat4, float DT, CNavigation* pNavigation)
{
    Vector4 vDir = XMLoadFloat4(&vFloat4);
    Vector4 vResutlPos{};
    if (pNavigation) vResutlPos = pNavigation->Is_MoveSliding(vDir, Get_State_ToCal(STATE::POSITION), m_fSpeedPerSec * DT);
    else vResutlPos = Get_State_ToCal(STATE::POSITION) + XMVector3Normalize(vDir) * m_fSpeedPerSec * DT;
    Set_State(STATE::POSITION, vResutlPos);

    /* 가져온 방향벡터를 정규화해서 내 속도만큼 더해준다 */
    Vector4 vResultPosition = Get_State_ToCal(STATE::POSITION) + XMVector3Normalize(XMLoadFloat4(&vFloat4)) * m_fSpeedPerSec * DT;
    if (pNavigation == nullptr || pNavigation->Is_Move(vResultPosition) == true) Set_State(STATE::POSITION, vResultPosition);

}
void CTransform::Move(const fVector4_1& vVector, float DT, CNavigation* pNavigation)
{
    Vector4 vResutlPos{};
    if(pNavigation) vResutlPos = pNavigation->Is_MoveSliding(vVector, Get_State_ToCal(STATE::POSITION), m_fSpeedPerSec * DT);
    else vResutlPos = Get_State_ToCal(STATE::POSITION) + XMVector3Normalize(vVector) * m_fSpeedPerSec * DT;
    Set_State(STATE::POSITION, vResutlPos);
}

#pragma endregion

#pragma region Move Rogic

bool CTransform::ChaseTarget(const fVector4_1& vTargetPos, float DT, float fMinDistance, CNavigation* pNavigation)
{
    /* 내 Position 값을 가져와 쫓아갈 타겟의 Pos 에서 내 Pos를 빼서 방향벡터를 만들어 준다.*/
    Vector4 MyPos = Get_State_ToCal(STATE::POSITION);
    Vector4 ChaseDir = vTargetPos - MyPos;

    /* 오차 검사 범위 내에 있으면 Return */
    if (XMVectorGetX(XMVector3Length(ChaseDir)) < fMinDistance)
        return true;


    /* 방향벡터를 정규화하여 내 포지션 값에 계속 더해준다. */
    Move(ChaseDir, DT, pNavigation);

    return false;
}

bool CTransform::ChaseTarget(const Float4& vTargetPos, float DT, float fMinDistance, CNavigation* pNavigation)
{
    Vector4 MyPos = Get_State_ToCal(STATE::POSITION);
    Vector4 TargetPos = XMLoadFloat4(&vTargetPos);
    Vector4 ChaseDir = TargetPos - MyPos;

    /* 오차 검사 범위 내에 있으면 Return */
    if (XMVectorGetX(XMVector3Length(ChaseDir)) < fMinDistance)
        return true;

    Move(ChaseDir, DT, pNavigation);

    return false;
}

bool CTransform::ChaseTarget_Offset(const Float4& vTargetPos, const Float3& vOffset, float DT, float fMinDistance, CNavigation* pNavigation)
{
    Vector4 MyPos = Get_State_ToCal(STATE::POSITION);
    Vector4 Offset = XMLoadFloat3(&vOffset);
    Vector4 TargetPos = XMLoadFloat4(&vTargetPos);


    Vector4 ChaseDir = (TargetPos + Offset) - MyPos;

    if (GetLengthVector(ChaseDir)) < fMinDistance) return true;

    Move(ChaseDir, DT, pNavigation);
    return false;
}

bool CTransform::ChaseTarget_Offset(const fVector4_1& vTargetPos, const Float3& vOffset, float DT, float fMinDistance, CNavigation* pNavigation)
{

    /* 내 Position 값을 가져와 쫓아갈 타겟의 Pos 에서 내 Pos를 빼서 방향벡터를 만들어 준다.*/
    Vector4 MyPos = Get_State_ToCal(STATE::POSITION);
    Vector4 Offset = XMLoadFloat3(&vOffset);

    Vector4 ChaseDir = (vTargetPos + Offset) - MyPos;

    /* 오차 검사 범위 내에 있으면 Return */
    if (GetLengthVector(ChaseDir)) < fMinDistance) return true;

    /* 방향벡터를 정규화하여 내 포지션 값에 계속 더해준다. */
    Move(ChaseDir, DT, pNavigation);

    return false;
}


/* Target지점까지 부드럽게 감속하는 움직임 */
bool CTransform::ChaseSmoothTarget(const fVector4_1& vTargetPos, float DT, float fMinDistance, CNavigation* pNavigation)
{
    Vector4 newPos = XMVectorLerp(GetPosition_ToCal(), vTargetPos, DT);

    if ( fabsf(XMVectorGetX(XMVector3Length(vTargetPos - newPos))) < fMinDistance)
        return true;

    if (pNavigation == nullptr || pNavigation->Is_Move(newPos) == true) Set_State(STATE::POSITION, newPos);

    
    return false;
}

bool CTransform::ChaseSmoothTarget(const Float4& vTargetPos, float DT, float fMinDistance, CNavigation* pNavigation)
{
    Vector4 newPos = XMVectorLerp(GetPosition_ToCal(), XMLoadFloat4(&vTargetPos), DT);

    if (fabsf(XMVectorGetX(XMVector3Length(XMLoadFloat4(&vTargetPos) - newPos))) < fMinDistance)
        return true;

    if (pNavigation == nullptr || pNavigation->Is_Move(newPos) == true) Set_State(STATE::POSITION, newPos);

    return false;
}


bool CTransform::ChaseSmoothTarget_Offset(const fVector4_1& vTargetPos, const Float3& vOffset, float DT, float fMinDistance,CNavigation* pNavigation)
{
    Vector4 newPos = XMVectorLerp(GetPosition_ToCal(), vTargetPos + XMLoadFloat3(&vOffset), DT);

    if (fabsf(XMVectorGetX(XMVector3Length(vTargetPos - newPos))) < fMinDistance)
        return true;

    if (pNavigation == nullptr || pNavigation->Is_Move(newPos) == true) Set_State(STATE::POSITION, newPos);


    return false;
}

bool CTransform::ChangeMatrix_ToTargetMatrix_Smooth(const XMatrix_1& TargetMatrix, float fRatio)
{
    XMVECTOR S_Target{}, R_Target{}, T_Target{}, S_My{}, R_My{}, T_My{};
    XMMatrixDecompose(&S_Target, &R_Target, &T_Target, TargetMatrix);
    XMMatrixDecompose(&S_My, &R_My, &T_My, XMLoadFloat4x4(&m_tWorldMatrix));

    /* 회전 행렬 Plus 위치값 천천히 이동... */
    XMVECTOR Lerp_T =  XMVectorLerp(T_My,T_Target,fRatio);
    XMVECTOR Lerp_Q = XMQuaternionSlerp(R_My, R_Target,fRatio);

    XMStoreFloat4x4(&m_tWorldMatrix, XMMatrixMultiply(
        XMMatrixMultiply(XMMatrixScalingFromVector(S_My), XMMatrixRotationQuaternion(Lerp_Q)), XMMatrixTranslationFromVector(XMVectorSetW(Lerp_T, 1.f))));

    return false;
}

void CTransform::MoveSmooth(const Float4& vDir, float DT, float fInterpolation)
{
    Vector4 MyPos = GetPosition_ToCal();

    Set_State(STATE::POSITION, XMVectorLerp(MyPos, MyPos + XMVector4Normalize(XMLoadFloat4(&vDir)) * DT , fInterpolation));
}

void CTransform::MoveSmooth(const fVector4_1& vDir, float DT,float fInterpolation)
{
    Vector4 MyPos = GetPosition_ToCal();

    Set_State(STATE::POSITION, XMVectorLerp(MyPos, MyPos + XMVector4Normalize(vDir) * DT, fInterpolation));
}

void CTransform::Move_SpeedDir(const Float3& vSpeedDir, float DT)
{
    Vector4 MyPos = GetPosition_ToCal();
    Vector4 vSpeed = XMLoadFloat3(&vSpeedDir);
    Vector4 vDir = XMVector3Normalize(vSpeed);
    Set_State(STATE::POSITION, XMVectorSetW( MyPos + (vDir * DT * XMVectorAbs(vSpeed)),1.f) );
}


#pragma endregion

#pragma region Rotation

/* 사원수 이용 */

/* 사원수 4개를 받아와서 사용되어질 예정 */

void CTransform::Apply_Rotation(const Matrix& matRotation)
{
    Float3 vScale = GetScale_ToStore();

    Vector4 vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    Vector4 vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    Vector4 vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, matRotation));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, matRotation));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, matRotation));
}

void CTransform::Rotation_Default(const Float3& vRot)
{
    m_tAccEuler = vRot;
    Apply_Rotation(XMMatrixRotationQuaternion(Get_QuaternionMyAccRot()));
}
void CTransform::Rotation_Default(const fVector4_1& vRot)
{
    m_tAccEuler = Float3( XMVectorGetX(vRot), XMVectorGetY(vRot) , XMVectorGetZ(vRot));

    Apply_Rotation(XMMatrixRotationQuaternion(Get_QuaternionMyAccRot()));
}

void CTransform::Rotation_Default(float fX, float fY, float fZ)
{
    /* 고정각도로 변환되면 초기화 */
    m_tAccEuler = Float3(fX,fY,fZ);

    Apply_Rotation(XMMatrixRotationQuaternion(Get_QuaternionMyAccRot()));
}

void CTransform::Rotation_AccMySpeed(ROT_FLAG eRotFlag, float DT)
{
    /* 쿼터니언 누적 변수는 라디안 기준 */
    if (eRotFlag & ROT_FLAG::ROT_X)
        m_tAccEuler.x +=  (DT * m_fRotationPerSec );
    if (eRotFlag & ROT_FLAG::ROT_Y)
        m_tAccEuler.y += (DT * m_fRotationPerSec);
    if (eRotFlag & ROT_FLAG::ROT_Z)
        m_tAccEuler.z += (DT * m_fRotationPerSec);

    Vector4 vQuat = Get_QuaternionMyAccRot();
    Matrix matRot = XMMatrixRotationQuaternion(vQuat);


    Float3 vScale = GetScale_ToStore();

    Vector4 vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    Vector4 vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    Vector4 vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    Set_State(STATE::RIGHT , XMVector3TransformNormal(vRight, matRot));
    Set_State(STATE::UP , XMVector3TransformNormal(vUp, matRot));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, matRot));
}

void CTransform::Rotation_AccMySpeed(const fVector4_1& vRotationSpeed, float DT)
{
    float SpeedRotX = XMVectorGetX(vRotationSpeed);
    float SpeedRotY = XMVectorGetY(vRotationSpeed);
    float SpeedRotZ = XMVectorGetZ(vRotationSpeed);

    if(SpeedRotX  != 0.f )
        m_tAccEuler.x += (DT * SpeedRotX);
    if(SpeedRotY != 0.f)
        m_tAccEuler.y += (DT * SpeedRotY);
    if(SpeedRotZ != 0.f)
        m_tAccEuler.z += (DT * SpeedRotZ);

    Vector4 vQuat = Get_QuaternionMyAccRot();
    Matrix matRot = XMMatrixRotationQuaternion(vQuat);

    Float3 vScale = GetScale_ToStore();

    Vector4 vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    Vector4 vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    Vector4 vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, matRot));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, matRot));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, matRot));
}


void CTransform::Rotation_AccRadian(float fX, float fY, float fZ)
{
    Vector4 MyRot = XMLoadFloat3(&m_tAccEuler);
    
    XMStoreFloat3(&m_tAccEuler, MyRot + XMVectorSet(fX, fY, fZ, 0.f));

    
    Vector4 vQuat = Get_QuaternionMyAccRot();
    Matrix  matRotation = XMMatrixRotationQuaternion(vQuat);

    Apply_Rotation(matRotation);
}

/* 들어올때 라디안 각도로 들어와야함 */
void CTransform::Rotation_AccRadian(const fVector4_1& vRot)
{
    //Vector4 MyRot = XMLoadFloat3(&m_tAccEuler);
    //MyRot = XMVectorSet(XMConvertToRadians(m_tAccEuler.x), XMConvertToRadians(m_tAccEuler.y), XMConvertToRadians(m_tAccEuler.z),0.f);
    //My

    //Vector4 vQuat = Get_QuaternionMyAccRot();
}

bool CTransform::Turn_To_TargetRotation_Smooth(const fVector4_1& vTargetRotation, float fLerp, float fMinRadian)
{
    Quaternion MyQuat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_tWorldMatrix));
    Quaternion LerpQuat = XMQuaternionSlerp(MyQuat,vTargetRotation,fLerp);

    XMMATRIX QuatMat = XMMatrixRotationQuaternion(LerpQuat);
    CTransform::Apply_Rotation(QuatMat);

    return false;
}




bool CTransform::LookTarget(const Float4& vTargetPos, float DT, float fMinAngle)
{
    Vector4 MyPos = Get_State_ToCal(Engine::STATE::POSITION);

    if (XMVectorGetX(XMVector3LengthSq(XMLoadFloat4(&vTargetPos) - MyPos)) <= 0.0001f)
        return true;


    Vector4 vecTargetPos = XMLoadFloat4(&vTargetPos);
    Vector4 vTargetLook = XMVector4Normalize( vecTargetPos - GetPosition_ToCal());

    /* Scale 값을 미리 저장해주기 */
    Float3 vScale = GetScale_ToStore();

    /* 외적을해서 회전축을 얻어보자 */
    /* 1. 임의의 Up벡터 (0 , 1 , 0) 을 만든다음 외적값을 통해 Right 벡터 얻기 */
    /* 2. 내가 봐야할 방향벡터 , 내가 바라보고 있는 방향벡터를 외적해서 Up벡터를 얻자 */
    
    /* 내가 바라볼 방향벡터에대한 나머지 Up , right 축을 구해주는 과정이다 */
    /* 초기 Up벡터를 0 1 0 으로 잡아준뒤 Right벡터를 구해주고 Up벡터를 구해주면 끝 */
    Vector4 vRight{};
    if (fabsf(XMVectorGetX(XMVector4Dot(vTargetLook, XMVectorSet(0.f, 1.f, 0.f, 0.f)))) < 0.99f)
        vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vTargetLook);
    else
        vRight = XMVector3Cross(XMVectorSet(0.f,0.f,1.f,0.f), vTargetLook);

    Vector4 vUp = XMVector3Cross( vTargetLook , vRight );

    /* 내가 바라볼 방향의 벡터에 대한 3개의 축을 외적을통해 구한거다 */


    Set_State(STATE::RIGHT,XMVector4Normalize(vRight) * vScale.x );
    Set_State(STATE::RIGHT,XMVector4Normalize(vUp) * vScale.y );
    Set_State(STATE::RIGHT,XMVector4Normalize(Get_State_ToCal(STATE::LOOK)) * vScale.z );

    return false;
}

bool CTransform::LookTarget(const fVector4_1& vTargetPos, float DT, float fMinAngle)
{
    Vector4 MyPos = Get_State_ToCal(Engine::STATE::POSITION);

    if (XMVectorGetX(XMVector3LengthSq(vTargetPos - MyPos)) <= 0.0001f)
        return true;

    Vector4 vTargetDir = XMVector4Normalize( vTargetPos - GetPosition_ToCal());
    Vector4 vNewRight{};
    Vector4 vNewUp{};

    Float3 vScale = GetScale_ToStore();

    if (fabsf(XMVectorGetX(XMVector3Dot(vTargetDir, XMVectorSet(0.f, 1.f, 0.f, 0.f)))) < 0.9999f)
        vNewRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vTargetDir);
    else
        vNewRight = XMVector3Cross(XMVectorSet(0.f, 0.f, 1.f, 0.f), vTargetDir);
    

    vNewUp = XMVector3Cross(vTargetDir,vNewRight);

    Set_State(STATE::RIGHT, XMVector3Normalize(vNewRight) * vScale.x);
    Set_State(STATE::UP, XMVector3Normalize(vNewUp) * vScale.y);
    Set_State(STATE::LOOK, XMVector3Normalize(vTargetDir) * vScale.z);

    Vector4 vLook = Get_State_ToCal(STATE::LOOK);

    float Angle = XMConvertToRadians(XMScalarACos(Vector3Dot( vTargetDir , vLook)));

    if (Angle < fMinAngle)
        return true;

    return false;
}

bool CTransform::LookTargetSmooth(const Float4& vTargetPos, float DT, float fMinAngle)
{
    Vector4 MyPos = Get_State_ToCal(Engine::STATE::POSITION);

    if (XMVectorGetX(XMVector3LengthSq(XMLoadFloat4(&vTargetPos) - MyPos)) <= 0.0001f)
        return true;

    Vector4 vMyLook = XMVector3Normalize(GetLook_ToCal());
    Vector4 vDirLook = XMVector3Normalize(XMLoadFloat4(&vTargetPos) - GetPosition_ToCal());

    float dot = XMVectorGetX(XMVector3Dot(vMyLook, vDirLook));
    dot = std::clamp(dot, -1.0f, 1.0f); // 안전용
    float angle = acosf(dot);                  // 라디안

    if (angle < fMinAngle)
    {
        return true;
    }

    /* 내 회전 쿼터니언 구하기 */
    Quaternion MyQuat{}, DirQuat{};
    MyQuat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_tWorldMatrix));

    if (fabsf(Vector3Dot(vMyLook, vDirLook)) < 0.9999f)
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirLook, Get_Up_Defualt())));
    else
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirLook, Get_Right_Defualt())));

    Matrix matRotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(MyQuat, DirQuat, DT * m_fRotationPerSec));

    Apply_Rotation(matRotation);

    return false;

}

bool CTransform::LookTargetSmooth(const fVector4_1& vTargetPos, float DT, float fMinAngle)
{
    Vector4 MyPos = Get_State_ToCal(Engine::STATE::POSITION);

    if (XMVectorGetX(XMVector3LengthSq(vTargetPos - MyPos)) <= 0.0001f)
        return true;

    Vector4 vMyLook  = XMVector3Normalize(GetLook_ToCal());
    Vector4 vDirLook = XMVector3Normalize( vTargetPos - GetPosition_ToCal());
    

   
    float dot = XMVectorGetX(XMVector3Dot(vMyLook, vDirLook));
    dot = std::clamp(dot, -1.0f, 1.0f); // 안전용
    float angle = acosf(dot);                  // 라디안

    if (angle < fMinAngle)
    {
        return true;
    }



    /* 내 회전 쿼터니언 구하기 */
    Quaternion MyQuat{}, DirQuat{};
    MyQuat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_tWorldMatrix));

    if (fabsf(Vector3Dot(vMyLook, vDirLook)) < 0.9999f) 
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirLook, Get_Up_Defualt())));
    else
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirLook, Get_Right_Defualt())));

    Matrix matRotation = XMMatrixRotationQuaternion( XMQuaternionSlerp(MyQuat , DirQuat , DT * m_fRotationPerSec));

    Apply_Rotation(matRotation);

    return false;
}

bool CTransform::LookDirectionSmooth(const fVector4_1& _vDirTarget, float DT, float fMinAngle)
{
    Vector4 vDirTarget = _vDirTarget;
    if (XMVectorGetX(XMVector3LengthSq(vDirTarget)) <= 0.0001f)
        return true;

    vDirTarget = XMVector3Normalize(vDirTarget);
    Vector4 vMyLook = XMVector3Normalize(GetLook_ToCal());

    float dot = XMVectorGetX(XMVector3Dot(vMyLook, vDirTarget));
    dot = std::clamp(dot, -1.0f, 1.0f);
    float angle = acosf(dot);

    if (angle < fMinAngle)
        return true; 

    Quaternion MyQuat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_tWorldMatrix));
    Quaternion DirQuat;

    if (fabsf(XMVectorGetY(vDirTarget)) > 0.99f)
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirTarget, Get_Right_Defualt())));
    else
        DirQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(XMMatrixLookToLH(XMVectorZero(), vDirTarget, Get_Up_Defualt())));

    Matrix matRotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(MyQuat, DirQuat, DT * m_fRotationPerSec));

    Apply_Rotation(matRotation);

    return false;
}



#pragma endregion

#pragma region  이동 회전 합친거

bool CTransform::ChaseAndLookTarget(bool isMoveSmooth, bool isRotationSmooth, const fVector4_1& vTargetPos, float DT, float fMinDistance, float fMinAngle,CNavigation* pNavigation )
{
    bool isGoal{ false };

    if (isMoveSmooth == true)
        if (isRotationSmooth == true)
        {
            isGoal = ChaseSmoothTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTargetSmooth(vTargetPos, DT, fMinDistance);
        }
        else
        {
            isGoal = ChaseSmoothTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTarget(vTargetPos, DT, fMinDistance);
        }
    else if (isMoveSmooth == false)
        if (isRotationSmooth == true)
        {
            isGoal = ChaseTarget(vTargetPos, DT, fMinDistance,pNavigation);
            LookTargetSmooth(vTargetPos, DT, fMinDistance);
        }
        else
        {
            isGoal = ChaseTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTarget(vTargetPos, DT, fMinDistance);
        }

    return isGoal;
}

bool CTransform::ChaseAndLookTarget(bool isMoveSmooth, bool isRotationSmooth, const Float4& vTargetPos, float DT, float fMinDistance, float fMinAngle, CNavigation* pNavigation)
{
    bool isGoal{ false };

    if (isMoveSmooth == true)
        if (isRotationSmooth == true)
        {
            isGoal =ChaseSmoothTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTargetSmooth(vTargetPos, DT, fMinDistance);
        }
        else
        {
            isGoal =ChaseSmoothTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTarget(vTargetPos, DT, fMinDistance);
        }
    else if (isMoveSmooth == false)
        if (isRotationSmooth == true)
        {
            isGoal = ChaseTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTargetSmooth(vTargetPos, DT, fMinDistance);
        }
        else
        {
            isGoal = ChaseTarget(vTargetPos, DT, fMinDistance, pNavigation);
            LookTarget(vTargetPos, DT, fMinDistance);
        }

    return isGoal;
}

void CTransform::Set_UIPosition(const VIEWPORT_DESC& tViewPortDesc, const UI_RATIO& tUIRatio)
{


    float fHalfW = tViewPortDesc.Width * 0.5f;
    float fHalfH = tViewPortDesc.Height * 0.5f;

    // 월드공간 중앙 기준 좌표
    float fX = fHalfW * tUIRatio.fRatioX;
    float fY = fHalfH * tUIRatio.fRatioY;

    float fSizeX = tViewPortDesc.Width * tUIRatio.fRatioSizeX;
    float fSizeY = tViewPortDesc.Height * tUIRatio.fRatioSizeY;

    Set_Scale(fSizeX, fSizeY, 1.f);
    Set_State(STATE::POSITION, XMVectorSet(fX, -fY, 0.5f, 1.f));
}


#pragma endregion




CTransform* CTransform::Create(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext)
{
    CTransform* pTransform = new CTransform(pDevice, pContext);

    if (FAILED(pTransform->Proto_Initialize()))
    {
        Safe_Release(pTransform);
        MSG_BOX("Transform is Failed to Create ");
        return nullptr;
    }

    return pTransform;
}


CComponent* CTransform::Clone(void* pArg)
{
    CTransform* pTransform = new CTransform(*this);

    if (FAILED(pTransform->Clone_Initialize(pArg)))
    {
        Safe_Release(pTransform);
        MSG_BOX("Transform is Failed to Create ");
        return nullptr;
    }

    return pTransform;
}

#ifdef _DEBUG

void CTransform::ImGuiUpdate()
{
    /* CTransform::ImGuiRender */
    /* Move , Rot Speed */
    if (ImGui::TreeNode("Speed"))
    {
        ImGui::InputFloat("Move Speed ", &m_fSpeedPerSec);
        ImGui::InputFloat("Rotation Speed", &m_fRotationPerSec);
        ImGui::TreePop();
    }
    
    bool isClick{ false };
    
    if (ImGui::TreeNode(" Transfrom Information "))
    {
        Float4 Pos{}, Rot{}, Scale{};
        Get_SRT(&Scale, &Rot, &Pos);
    
        Rot.x = XMConvertToDegrees(Rot.x);
        Rot.y = XMConvertToDegrees(Rot.y);
        Rot.z = XMConvertToDegrees(Rot.z);
    
    
        if (ImGui::TreeNode(" Position "))
        {
            CImGuiFuntion::ColorText(COLOR_RED, "Position");
            ImGui::Text(" X : %.2f ", Pos.x);ImGui::SameLine();
            if (ImGui::InputFloat("X##1", &m_tWorldMatrix._41, 0.1f)) { isClick = true; };
    
            ImGui::Text(" Y : %.2f ", Pos.y);ImGui::SameLine();
            if (ImGui::InputFloat("Y##1", &m_tWorldMatrix._42, 0.1f)) { isClick = true; };
    
            ImGui::Text(" Z : %.2f ", Pos.z); ImGui::SameLine();
            if (ImGui::InputFloat("Z##1", &m_tWorldMatrix._43, 0.1f)) { isClick = true; };
    
    
            if (isClick == true)
                static_cast<CGameObject*>(m_pOwner)->Semi_Update();

    
            ImGui::TreePop();
        }
    
        if (ImGui::TreeNode(" Scale "))
        {
    
            ImGui::Text(" X : %.2f ", m_vChangeScale.x);
            if (ImGui::InputFloat("X##2" , &m_vChangeScale.x, 0.1f)) { isClick = true; }
    
            ImGui::Text(" Y : %.2f ", m_vChangeScale.y);
            if (ImGui::InputFloat("Y##2", &m_vChangeScale.y, 0.1f)) { isClick = true; }
    
            ImGui::Text(" Z : %.2f ", m_vChangeScale.z);
            if (ImGui::InputFloat("Z##2", &m_vChangeScale.z, 0.1f)) { isClick = true; }
    
            if (isClick == true)
            {
                Set_Scale(m_vChangeScale);
                static_cast<CGameObject*>(m_pOwner)->Semi_Update();
            }
    
    
            ImGui::TreePop();
        }
    
    
        if (ImGui::TreeNode(" Rotation "))
        {
            ImGui::Text(" X : %.2f ", m_vChangeRotation.x);
            if (ImGui::InputFloat("X##3", &m_vChangeRotation.x, 0.1f)) { isClick = true; }
            
            ImGui::Text(" Y : %.2f ", m_vChangeRotation.y);
            if (ImGui::InputFloat("Y##3", &m_vChangeRotation.y, 0.1f)) { isClick = true; }
    
            ImGui::Text(" Z : %.2f ", m_vChangeRotation.z);
            if (ImGui::InputFloat("Z##3", &m_vChangeRotation.z, 0.1f)) { isClick = true; }
    
    
            if (isClick == true)
            {
                Rotation_Default( XMVectorSet( XMConvertToRadians(m_vChangeRotation.x), XMConvertToRadians(m_vChangeRotation.y), XMConvertToRadians(m_vChangeRotation.z),0.f));
                static_cast<CGameObject*>(m_pOwner)->Semi_Update();
            }
    
            ImGui::TreePop();
        }
    
    
        if (ImGui::TreeNode("World Matrix"))
        {
            CImGuiFuntion::RenderMatrix(m_tWorldMatrix);
            ImGui::TreePop();
        }
    
  
        ImGui::TreePop();
    }
}
#endif

Matrix CTransform::Get_WorldMatrixInverse() CONST
{
    return XMMatrixInverse(nullptr ,XMLoadFloat4x4(&m_tWorldMatrix)); 
}



void CTransform::Free()
{
    __super::Free();

    /* My Free() */
}

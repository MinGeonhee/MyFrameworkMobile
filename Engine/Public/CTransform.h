#pragma once
#include "CComponent.h"

NS_BEGIN(Engine)

enum ROT_FLAG
{
	ROT_X = 1 << 0,
	ROT_Y = 1 << 1,
	ROT_Z = 1 << 2,
};

enum class ROTATION_TYPE
{
	DEFAULT = 0,
	ACC,
	END,
};


/*
계산용 Vector의 각 성분을 추출해서 반환해주는 함수이다. 단순 float 을 반환해준다.
XMVectorGetX , XMVectorGetY , XMVectorGetZ , XMVectorGetW

*/
class CGameObject;
class CNavigation;

class ENGINE_DLL CTransform final :    public CComponent
{
protected:
	explicit			CTransform(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit			CTransform(const CTransform& Prototype);
	virtual				~CTransform();
private:
	HRESULT				Proto_Initialize()						override;
	HRESULT				Clone_Initialize(void* pArg)			override;
public:

#ifdef _DEBUG
	virtual void		ImGuiUpdate()							override;
#endif
	const Float4x4&		Get_matWorld()				const { return m_tWorldMatrix; }
	Matrix				Get_matWorld_ToCal()		const { return XMLoadFloat4x4(&m_tWorldMatrix); }
	Matrix				Get_WorldMatrixInverse()	CONST;
	const Float4x4*		Get_matWorldPtr()			const { return &m_tWorldMatrix; }


public:
#pragma region State Get Set
	/* 저장용 FLoat4 를 계산용으로 전환하여 반환해주는 함수 */
	Vector4				Get_State_ToCal(STATE eState)const;
	Float4				Get_State_ToStore(STATE eState)const;

	/*  저장용 벡터를 가져와서 바로 저장시켜버리기 */
	void				Set_State(STATE eState, const Float4& vFloat4);
	/* 계산용 벡터를 치환후 저장시킬 예정 */
	void				Set_State(STATE eState, const fVector4_1& vVector4);

	void				Set_Position(const fVector4_1& vPos , CNavigation* pNavigation = nullptr);

	void				Set_WorldMatrix(const Float4& vRight, const Float4& vUp, const Float4& vLook, const Float4& vPos);
	void				Set_WorldMatrix(const fVector4_1& vRight, const fVector4_2& vUp, const fVector4_3& vLook, const GVector4_4& vPos);
	void				Set_WorldMatrix(const XMatrix_1& matWorld);

	void				Get_SRT(Float4* vScale, Float4* vRot, Float4* vTrans);


	float				Get_Length(const Float4& vPos)const;
	float				Get_Length(const fVector4_1& vPos)const;

	float				Get_LengthSq(const Float4& vPos)const;
	float				Get_LengthSq(const fVector4_1& vPos)const;

#pragma endregion


	bool				Get_IsInCamArea(const Float4& vRenderCamAreaXZ);


#pragma region  스케일 관련

public:
	Vector4				GetScale_ToCal()const;
	Float3				GetScale_ToStore()const;

	float				GetScaleX()const;
	float				GetScaleY()const;
	float				GetScaleZ()const;
	/* Set Scale */
	void				Set_Scale(const fVector4_1& vScale , CALCULATE_TYPE eCalType = CALCULATE_TYPE::DEFAULT);
	void				Set_Scale(const Float3&		vScale , CALCULATE_TYPE eCalType = CALCULATE_TYPE::DEFAULT);
	void				Set_Scale(float fX, float fY, float fZ , CALCULATE_TYPE eCalType = CALCULATE_TYPE::DEFAULT);
#pragma endregion


#pragma region 거리비교

	bool				Check_InDistanceRange(const fVector4_1& vTargetPos,float fRange);


#pragma region 이동관련
public:


	void				Go_Straight(float DT,CNavigation* pNavigation = nullptr);
	void				Go_Back(float DT);
	void				Go_Left(float DT);
	void				Go_Right(float DT);

	void				Go_Up(float DT);
	void				Go_Down(float DT);

	/* 방향벡터를 매개변수로 받아서 그 방향으로 이동시켜준다. */
	void				Move(const Float4& vFloat4, float DT, CNavigation* pNavigation = nullptr);
	void				Move(const fVector4_1& vVector, float DT, CNavigation* pNavigation = nullptr);
	void				MoveSmooth(const Float4& vDir, float DT, float finterpolation = 0);
	void				MoveSmooth(const fVector4_1& vDir, float DT,float finterpolation = 0);
	
	void				Move_SpeedDir(const Float3& vSpeedDir, float DT );

	/* 보통 벡터로 받아와서 넣어주는 Float으로 넣었을시는... */
	bool				ChaseTarget(const fVector4_1& vTargetPos , float DT , float fMinDistance = 0, CNavigation* pNavigation = nullptr);
	bool				ChaseTarget(const Float4& vTargetPos , float DT , float fMinDistance = 0, CNavigation* pNavigation = nullptr);
	bool				ChaseTarget_Offset(const Float4& vTargetPos , const Float3& vOffset , float DT ,float fMinDistance = 0, CNavigation* pNavigation = nullptr);
	bool				ChaseTarget_Offset(const fVector4_1& vTargetPos, const Float3& vOffset, float DT  ,float fMinDistance = 0, CNavigation* pNavigation = nullptr);

	bool				ChaseSmoothTarget(const fVector4_1& vTargetPos, float DT , float fMinDistance = 0, CNavigation* pNavigation = nullptr);
	bool				ChaseSmoothTarget(const Float4& vTargetPos, float DT , float fMinDistance = 0, CNavigation* pNavigation = nullptr);
	bool				ChaseSmoothTarget_Offset(const fVector4_1& vTargetPos, const Float3& vOffset ,float DT , float fMinDistance = 0, CNavigation* pNavigation = nullptr);

	bool				ChangeMatrix_ToTargetMatrix_Smooth(const XMatrix_1& TargetMatrix, float fRatio);


#pragma endregion 

#pragma region 회전 관련

public:
	void				Apply_Rotation(const Matrix& matRotation);

public:
	void				Rotation_Default(const Float3& vRot);
	void				Rotation_Default(const fVector4_1& vRot);
	void				Rotation_Default(float fX, float fY, float fZ);

	/*  돌릴각도를 플래그 값으로 넣어주면 된다*/
	void				Rotation_AccMySpeed( ROT_FLAG eRotFlag , float DT);
	void				Rotation_AccMySpeed( const fVector4_1& vRotationSpeed  , float DT);

	void				Rotation_AccRadian(float fX, float fY, float fZ);
	void				Rotation_AccRadian(CONST fVector4_1& vRot);
	bool				Turn_To_TargetRotation_Smooth(const fVector4_1& vTargetRotation ,float fLerp , float fMinRadian = 0.01f );

	bool				LookTarget(const Float4& vTargetPos , float DT , float fMinRadian = 0);
	bool				LookTarget(const fVector4_1& vTargetPos , float DT , float fMinRadian = 0);

	bool				LookTargetSmooth(const Float4& vTargetPos, float DT, float fMinRadian = 0);
	bool				LookTargetSmooth(const fVector4_1& vTargetPos, float DT, float fMinRadian = 0);

	/* 단순 로테이션 함수 */
	bool				LookDirectionSmooth(const fVector4_1& _vDirTarget, float DT, float fMinAngle);

#pragma endregion

#pragma region 이동 회전 합친거
	bool				ChaseAndLookTarget(bool isMoveSmooth , bool isRotationSmooth,const fVector4_1& vTargetPos, float DT, float fMinDistance = 0 , float fMinAngle = 0, CNavigation* pNavigation =nullptr);
	bool				ChaseAndLookTarget(bool isMoveSmooth , bool isRotationSmooth,const Float4& vTargetPos, float DT, float fMinDistance = 0 , float fMinAngle = 0, CNavigation* pNavigation = nullptr);
#pragma endregion
public:
	const float&		Get_RotationPerSec()	const	{ return m_fRotationPerSec; }
	const float&		Get_SpeedPerSec()		const	{ return m_fSpeedPerSec; }

	void				Set_RotationPerSec(float fPerSec) { m_fRotationPerSec = fPerSec; }
	void				Set_SpeedPerSec(float fPerSec)		{ m_fSpeedPerSec = fPerSec; }

#pragma region

public:
	void				Set_UIPosition(CONST VIEWPORT_DESC& tViewPortDesc , CONST UI_RATIO& tUIInfo);

#pragma endregion


private:
	Vector4				Get_QuaternionMyAccRot() { return XMQuaternionRotationRollPitchYaw(m_tAccEuler.x, m_tAccEuler.y, m_tAccEuler.z); }
public:

	const XMFLOAT3&		Get_AccEuler()	{ return m_tAccEuler; }
private:
	/* Scale 은 Left , Up , Look 벡터의 각 길이*/
	/* 회전정보를 알고 싶다  -> 회전시킬때마다 누적해주는 변수를 만들어준다 */
	/* 정확하게 회전한 값을 얻을 수 없기에 변수를 하나마련해서 누적시키는게 좋다. */

	float				m_fRotationPerSec;
	float				m_fSpeedPerSec;
	Float4x4			m_tWorldMatrix;

	
	XMFLOAT3			m_tAccEuler;

	float				m_fAccTime;
	/* 짐벌락 현상 : 무조건 나타는 현상이아니라 특수조건을 만족하면 나타나는 현상이다  */
	

#ifdef _DEBUG

	Float3				m_vChangeScale{};
	Float3				m_vChangeRotation{};

#endif // _DEBUG



public:
	static CTransform*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CComponent*			Clone(void* pArg)	override;
	void				Free() override;
};

NS_END
#include "CUI_Animation.h"
#include "CUITransform.h"
#include "CUI_Image.h"

CUI_Animation::CUI_Animation(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CUIComponent(pDevice,pContext), m_pOwnerUIImageCom(nullptr) , m_pOwnerUITransform(nullptr) , m_isPlayingAnim(false)
	, m_eAnimType(CUI_Animation::TYPE::NONE), m_fAccTime(0.f), m_fSetTime(0.f), m_fSpeed(0.f), m_AnimEndFunction{}
	, m_isMove(false) ,m_isSmoothMove(false) , m_eMoveType(CUI_Animation::MOVE_TYPE::END)
	, m_vTargetPos{ 0.f,0.f }, m_fMoveSpeed{ 0.f }, m_fMoveSetTime{}, m_fMoveAccTime{} , m_MoveEndFunction(nullptr)
	, m_vStartPos{}, m_fPower{}
{
}

CUI_Animation::CUI_Animation(const CUI_Animation& Proto)
	: CUIComponent(Proto) , m_pOwnerUIImageCom(Proto.m_pOwnerUIImageCom), m_pOwnerUITransform(Proto.m_pOwnerUITransform), m_isPlayingAnim(Proto.m_isPlayingAnim)
	, m_eAnimType(Proto.m_eAnimType) , m_fAccTime(Proto.m_fAccTime) , m_fSetTime(Proto.m_fSetTime) , m_fSpeed(Proto.m_fSpeed), m_AnimEndFunction{Proto.m_AnimEndFunction }
	, m_isMove(Proto.m_isMove), m_isSmoothMove(Proto.m_isSmoothMove), m_eMoveType(Proto.m_eMoveType)
	, m_vTargetPos{ Proto.m_vTargetPos }, m_fMoveSpeed{ Proto.m_fMoveSpeed }, m_fMoveSetTime{ Proto.m_fMoveSetTime}, m_fMoveAccTime{ Proto.m_fMoveAccTime}
	, m_MoveEndFunction(Proto.m_MoveEndFunction)
	, m_vStartPos(Proto.m_vStartPos)
	, m_fPower(Proto.m_fPower)
{
}

CUI_Animation::~CUI_Animation()
{
}

HRESULT	CUI_Animation::Proto_Initialize()
{
	if (FAILED(__super::Proto_Initialize()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_Animation::Clone_Initialize(void* pArg)
{
	CUI_Animation::UI_ANIAMTION_DESC* pDesc = static_cast<CUI_Animation::UI_ANIAMTION_DESC*>(pArg);
	
	m_pOwnerUIImageCom  = pDesc->pOwnerUIImageCom;
	m_pOwnerUITransform = pDesc->pOwnerUITransform;

	return S_OK;
}

void CUI_Animation::Anim_Clear()
{
	m_fAccTime = 0.f;
	m_fSetTime = 0.f;
	m_isPlayingAnim = false;
	m_eAnimType = CUI_Animation::TYPE::NONE;
}

void CUI_Animation::Pre_Update(float DT)
{
	__super::Pre_Update(DT);

	return;
}

void CUI_Animation::Update(float DT)
{
	//__super::Update(DT);

	if (m_isPlayingAnim)
	{
		if (!m_pOwnerUIImageCom) return;


		switch (m_eAnimType)
		{
		case Engine::CUI_Animation::TYPE::NONE:								return;
		case Engine::CUI_Animation::TYPE::FADE_IN:		Update_FadeIn(DT);	return;
		case Engine::CUI_Animation::TYPE::FADE_OUT:		Update_FadeOut(DT); return;
		case Engine::CUI_Animation::TYPE::BLINK:		Update_Blink(DT);	return;
		default:															return;
		}
	}

	if (m_isMove)
	{
		if (!m_pOwnerUITransform) return;

		switch (m_eMoveType)
		{
		case Engine::CUI_Animation::MOVE_TYPE::MoveTarget:	Update_MoveTarget(DT);	return;
		case Engine::CUI_Animation::MOVE_TYPE::Bounce:		Update_Bounce(DT);		return;
		case Engine::CUI_Animation::MOVE_TYPE::END:									return;
		default:																	return;
		}
	}

	return;
}

void CUI_Animation::Late_Update(float DT)
{
	__super::Late_Update(DT);
}

HRESULT CUI_Animation::Render(CCanvas* pCanvas)
{
	if (!m_isPlayingAnim) return S_OK;


	return S_OK;
}

void CUI_Animation::Update_FadeIn(float DT)
{
	if (!m_pOwnerUIImageCom) return;

	/* 선형보간해서 알파값을 낮춘뒤 Image Component에 Setting 해주기 */
	/* Fade In => 알파값이 0 -> 1 로 들어가야한다 */
	float fLerpAlpha = Lerp(0,1,(m_fAccTime+=DT/m_fSetTime));

	/* 알파값은 1 보다 높을 수 없다 */
	if (fLerpAlpha >= 1)
	{
		fLerpAlpha = 1.f;
		Anim_Clear();

		if (m_AnimEndFunction)
		{
			m_AnimEndFunction();
			m_AnimEndFunction = nullptr;
		}
	}
	/* UIImage의 알파값을 세팅해준다 */
	m_pOwnerUIImageCom->m_fAlpha = fLerpAlpha;
}

void CUI_Animation::Update_FadeOut(float DT)
{
	/* 선형보간해서 알파값을 낮춘뒤 Image Component에 Setting 해주기 */
	/* Fade Out => 알파값이 1 -> 0 로 들어가야한다 */
	float fLerpAlpha = Lerp(0,1,(m_fAccTime -= DT / m_fSetTime));

	/* 알파값은 0 보다 낮을수없다 */
	if (fLerpAlpha <= 0.f)
	{
		fLerpAlpha = 0.f;
		Anim_Clear();
		
		if(m_AnimEndFunction)
		{ 
			m_AnimEndFunction();
			m_AnimEndFunction= nullptr;
		}
	}

	/* UIImage의 알파값을 세팅해준다 */
	m_pOwnerUIImageCom->m_fAlpha = fLerpAlpha;
}

void CUI_Animation::Update_Blink(float DT)
{
	if (!m_pOwnerUIImageCom) return;


	/* 깜빡 깜빡 거리는 효과 */
	/* sin파를 이용하자... */

	float fAlpha = sinf( ( m_fAccTime+=DT ) * m_fSpeed) * 0.5f + 0.5f;
	//m_pOwnerUIImageCom->m_fAlpha = fAlpha;
}

void CUI_Animation::Update_MoveTarget(float DT)
{
	Vector4 vMyPos = XMLoadFloat2(&m_pOwnerUITransform->Get_Pos());
	Vector4 vDir =  XMVector3Normalize(XMLoadFloat2(&m_vTargetPos) - vMyPos);

	m_fMoveAccTime += DT;

	if (m_fMoveAccTime >= m_fMoveSetTime)
	{
		Reset_MoveEvent();
		return;
	}

	if (m_isSmoothMove)
	{
		/* 선형보관해야한다 */
		float fRadio = m_fMoveAccTime / m_fMoveSetTime;

		if (fRadio > 1.f)
			fRadio = 1.f;
		Vector4 vCurPos =XMVectorLerp(XMLoadFloat2(&m_vStartPos), XMLoadFloat2(&m_vTargetPos), fRadio);

		m_pOwnerUITransform->Set_Pos(Float2(XMVectorGetX(vCurPos),XMVectorGetY(vCurPos)));
	}
	else
	{
		vMyPos += vDir * DT * m_fMoveSpeed;
		m_pOwnerUITransform->Set_Pos(Float2(XMVectorGetX(vMyPos),XMVectorGetY(vMyPos)));
	}
}

void CUI_Animation::Update_Bounce(float DT)
{
	m_fMoveAccTime += m_fMoveSpeed * DT;

	float fSinValue = sinf(m_fMoveAccTime);
	
	if (fSinValue < 0.f)
	{
		Reset_MoveEvent();
		m_pOwnerUITransform->Set_Pos(m_vStartPos);
		return;
	}

	float fOffsetY = m_fPower * fSinValue;
	
	m_pOwnerUITransform->Set_Pos(Float2(m_vStartPos.x , m_vStartPos.y + fOffsetY));
}

void CUI_Animation::Reset_MoveEvent()
{
	/* 나머지 초기화 */
	m_fMoveSpeed = 0.f;
	m_fMoveSetTime = 0.f;
	m_fMoveAccTime = 0.f;
	m_vTargetPos = { 0.f,0.f };
	m_isSmoothMove = false;
	m_isMove = false;
	if (m_MoveEndFunction)
	{
		m_MoveEndFunction();
		m_MoveEndFunction = nullptr;
	}
}


void CUI_Animation::Fade_In(float fFadeTime,function<void()> EndFunc)
{
	if (!m_pOwnerUIImageCom) return;
	//if (m_isPlayingAnim) return;

	
	m_fAccTime = 0.f;
	m_fSetTime = fFadeTime;
	m_pOwnerUIImageCom->Set_Aplha(0.f);

	m_AnimEndFunction = EndFunc;

	m_eAnimType = CUI_Animation::TYPE::FADE_IN;
	m_isPlayingAnim = true;
}

void CUI_Animation::Fade_Out(float fFadeTime,function<void()> EndFunc)
{
	NULLPTR_RETURN_NULL(m_pOwnerUIImageCom);
	//if (m_isPlayingAnim) return;

	m_fAccTime = fFadeTime;
	m_fSetTime = fFadeTime;
	m_pOwnerUIImageCom->Set_Aplha(1.f);

	m_AnimEndFunction = EndFunc;

	m_eAnimType = CUI_Animation::TYPE::FADE_OUT;
	m_isPlayingAnim = true;
}


void CUI_Animation::Blink(float fBlinkSpeed)
{
	NULLPTR_RETURN_NULL(m_pOwnerUIImageCom);
	if (m_isPlayingAnim) return;

	m_fSpeed = fBlinkSpeed;

	m_eAnimType = CUI_Animation::TYPE::BLINK;
	m_isPlayingAnim = true;
}

void CUI_Animation::Bounce(float fBoundPower, float fSpeed, function<void()> MoveEndFunc)
{
	NULLPTR_RETURN_NULL(m_pOwnerUITransform);
	if (m_isMove == true) return;

	m_eMoveType = CUI_Animation::MOVE_TYPE::Bounce;
	m_isMove = true;
	m_vStartPos = m_pOwnerUITransform->Get_Pos();
	m_vTargetPos = Float2(m_vStartPos.x, m_vStartPos.y + fBoundPower);
	m_MoveEndFunction = MoveEndFunc;
	m_fPower = fBoundPower;
	m_fMoveSpeed = fSpeed;
}

void CUI_Animation::MoveTarget(const Float2& vTargetPos, float fTime ,bool isSmooth, function<void()> MoveEndFunc)
{
	/*  처음에 내가 로컬좌표로 움직이기 때문에 */
	NULLPTR_RETURN_NULL(m_pOwnerUITransform);
	if (m_isMove == true) return;


	m_fMoveSetTime = fTime;


	m_vTargetPos = vTargetPos;
	m_isSmoothMove = isSmooth;
	m_isMove = true;
	m_eMoveType = CUI_Animation::MOVE_TYPE::MoveTarget;
	
	m_vStartPos = m_pOwnerUITransform->Get_Pos();
	float fLength = fabsf(XMVectorGetX(XMVector2Length(XMLoadFloat2(&vTargetPos) - XMLoadFloat2(&m_vStartPos))));

	m_fMoveSpeed = fLength / m_fMoveSetTime;

	m_MoveEndFunction = MoveEndFunc;

	return;
}

CUI_Animation* CUI_Animation::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CUI_Animation* pUIAnimation = new CUI_Animation(pDevice, pContext);

	if (FAILED(pUIAnimation->Proto_Initialize()))
	{
		Safe_Release(pUIAnimation);
		MSG_BOX("UI Animation  is failed to Create");
		return nullptr;
	}

	return pUIAnimation;
}


CComponent* CUI_Animation::Clone(void* pArg)
{
	CUI_Animation* pUIAnimation = new CUI_Animation(*this);

	if (FAILED(pUIAnimation->Clone_Initialize(pArg)))
	{
		Safe_Release(pUIAnimation);
		MSG_BOX("UI Animation is failed to Clone");
		return nullptr;
	}

	return pUIAnimation;
}


void CUI_Animation::Free()
{
	__super::Free();
}

#pragma once
#include "CUIComponent.h"


namespace Engine
{
	class CUI_Image;
	class CUITransform;
}

NS_BEGIN(Engine)

class ENGINE_DLL CUI_Animation final : public CUIComponent
{
public:
	enum class TYPE
	{
		NONE , FADE_IN , FADE_OUT , BLINK , END
	};
	enum class MOVE_TYPE
	{
		MoveTarget, Bounce , END
	};
public:
	typedef struct UI_Aniamtion_Desc : public CUIComponent::UICOMPONENT_DESC
	{
		CUI_Image*			pOwnerUIImageCom{nullptr};
		CUITransform*		pOwnerUITransform{nullptr};
	}UI_ANIAMTION_DESC;
private:
	explicit		CUI_Animation(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit		CUI_Animation(const CUI_Animation& Proto);
	virtual			~CUI_Animation();
private:
	HRESULT			Proto_Initialize();
	virtual HRESULT Clone_Initialize(void* pArg);

private:
	void			Anim_Clear();
public:
	virtual void    Pre_Update(float DT)	override;
	virtual void    Update(float DT)		override;
	virtual void    Late_Update(float DT)	override;
	virtual HRESULT Render(CCanvas* pCanvas)override;

private:
	void			Update_FadeIn(float DT);
	void			Update_FadeOut(float DT);
	void			Update_Blink(float DT);
public:
	void			Update_MoveTarget(float DT);
	void			Update_Bounce(float DT);

private:
	void			Reset_MoveEvent();
public:
	/* Fade In Out */
	void			Fade_In(float fFadeTime, function<void()> EndFunc =nullptr);
	void			Fade_Out(float fFadeTime, function<void()> EndFunc = nullptr);
	void			Blink(float fBlinkSpeed);

public:
	void			Bounce( float fBoundPower , float fSpeed , function<void()> MoveEndFunc = nullptr);
	void			MoveTarget(const Float2& vTargetPos, float fTime,bool isSmooth = false ,function<void()> MoveEndFunc = nullptr);
public:
	void			Set_OwnerUIImage(CUI_Image* pUIImageCom)			{ m_pOwnerUIImageCom = pUIImageCom; }
	void			Set_OwnerUITransform(CUITransform* pUITransformCom) { m_pOwnerUITransform = pUITransformCom; }
public:
	CUI_Image*		Get_OwnerUIImage()		 const { return m_pOwnerUIImageCom; }
	CUITransform*	Get_OwnerUITransform()	 const { return m_pOwnerUITransform; }
	bool			Get_IsPlayingAnim()		 const { return m_isPlayingAnim; }

public:
	bool				 m_isPlayingAnim;
	CUI_Animation::TYPE	 m_eAnimType;
	float				 m_fSpeed;
	float				 m_fSetTime;
	float				 m_fAccTime;

	std::function<void()> m_AnimEndFunction;

	/* 내 주읜 Transform */
	CUI_Image*			 m_pOwnerUIImageCom;
	CUITransform*		 m_pOwnerUITransform;
	float				 m_fMoveSpeed;
	float			     m_fMoveSetTime;
	float				 m_fMoveAccTime;
	std::function<void()> m_MoveEndFunction;
	Float2				m_vStartPos;
	float			     m_fPower;



	CUI_Animation::MOVE_TYPE	 m_eMoveType;
	bool						 m_isMove;
	bool						 m_isSmoothMove;
	Float2						 m_vTargetPos;
public:
	static CUI_Animation*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CComponent*				Clone(void* pArg) override;
	virtual void			Free() override;
};


NS_END

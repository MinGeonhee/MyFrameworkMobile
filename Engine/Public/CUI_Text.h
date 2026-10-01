#pragma once
#include "CUIObject.h"

/* Text에다가 Type을 지정할 수 있게 한다음에 지정되 Type으로 렌더함수부르기? */




namespace Engine
{
	class CFont;
	class CUI_Animation;
}




NS_BEGIN(Engine)


class ENGINE_DLL CUI_Text final : public CUIObject
{
public:
	typedef struct UI_Text_Desc : public CUIObject::UIOBJECT_DESC
	{
		FONT_RENDER_TYPE eFontRenderType{FONT_RENDER_TYPE::DEFUALT};
		Color			 vColor{};
		std::wstring	 wstrFindFontKey{};
		std::wstring	 wstrText{};
	}UI_TEXT_DESC;
private:
	CUI_Text(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CUI_Text() {}
private:
	HRESULT	Initialize(UI_TEXT_DESC* pDesc);
	HRESULT	Ready_Component(UI_TEXT_DESC* pDesc);
public:
	virtual void			Pre_Update(CCanvas* pCanvas, float DT)		override;
	virtual void			Update(CCanvas* pCanvas, float DT)			override;
	virtual void			Late_Update(CCanvas* pCanvas, float DT)		override;
	virtual HRESULT			Render(CCanvas* pCanvas)					override;
public:
	void					Set_Text(wstring wstrText) { m_wstrText = wstrText; }
	void					Set_FontRenderType(FONT_RENDER_TYPE eType) { m_eFontRenderType = eType; }
public:
	FONT_RENDER_TYPE		Get_Font_Render_Type() const	{ return m_eFontRenderType; }
	CFont*					Get_Font() const				{ return m_pFont;}
	wstring					Get_Text() const				{ return m_wstrText; }
	CUI_Animation*			Get_UIAnimaiton() const			{ return m_pUIAnimationCom; }
public:
	FONT_RENDER_TYPE		m_eFontRenderType;
	CUI_Animation*			m_pUIAnimationCom;
	Color					m_vColor;
	SpriteBatch*			m_pSpriteBatch;
	CFont*					m_pFont;
	wstring					m_wstrText;
public:
	static CUI_Text* Create(UI_TEXT_DESC* pDesc,ComPtr<Device> pDevice,ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END


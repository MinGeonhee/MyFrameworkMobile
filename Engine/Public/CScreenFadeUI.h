#pragma once
#include "CUIObject.h"

namespace Engine
{
	class CUI_Image;
	class CUI_Animation;
}

NS_BEGIN(Engine)

class ENGINE_DLL CScreenFadeUI : public CUIObject
{
protected:
	explicit CScreenFadeUI(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CScreenFadeUI() = default;
protected:
	HRESULT					Initialize();
	virtual HRESULT			Ready_Component();
public:
	virtual void			Pre_Update(CCanvas* pCanvas, float DT);
	virtual void			Update(CCanvas* pCanvas, float DT);
	virtual void			Late_Update(CCanvas* pCanvas, float DT);
	virtual HRESULT			Render(CCanvas* pCanvas);
public:
	void					Fade_In(float fTime);
	void					Fade_Out(float fTime);
private:
	CUI_Image*				m_pUIImageCom;
	CUI_Animation*			m_pUIAnimationCom;
public:
	static  CScreenFadeUI*	Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void			Free() override;
};

NS_END


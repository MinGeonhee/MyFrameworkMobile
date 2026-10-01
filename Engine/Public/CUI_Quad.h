#pragma once
#include "CUIObject.h"

namespace Engine
{
	class CUI_Animation;
	class CUI_Image;
}

NS_BEGIN(Engine)

class ENGINE_DLL CUI_Quad final:  public CUIObject
{
public:
	typedef struct UI_Quad_Desc : public CUIObject::UIOBJECT_DESC
	{
		wstring     wstrTextureKey{};
		int32		iTextureIndex{ 0 };
	}UI_QUAD_DESC;
public:
	explicit CUI_Quad(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CUI_Quad() {}
protected:
	HRESULT					Initialize(UI_QUAD_DESC* pDesc);
	HRESULT					Ready_Component(UI_QUAD_DESC* pDesc);
public:
	void					Set_TextureIndex(int32 iIndex);
	void					Set_Alpha(float fAlpha);
public:
	int32					Get_TextureIndex() const;
public:
	CUI_Animation*			Get_UIAnimation() const { return m_pUIAnimationCom;}
	CUI_Image*				Get_Iamge()const		{ return m_pUIImageCom; }
public:
	virtual void			Pre_Update(CCanvas* pCanvas, float DT)override;
	virtual void			Update(CCanvas* pCanvas, float DT)override;
	virtual void			Late_Update(CCanvas* pCanvas, float DT)override;
	virtual HRESULT			Render(CCanvas* pCanvas)override;
private:
	CUI_Animation*			m_pUIAnimationCom;
	CUI_Image*				m_pUIImageCom;
public:
	static CUI_Quad* Create(UI_QUAD_DESC* pDesc, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void			Free();
};

NS_END
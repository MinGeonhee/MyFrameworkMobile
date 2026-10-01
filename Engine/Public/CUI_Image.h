#pragma once
#include "CUIComponent.h"
#include "CUI_Quad.h"


namespace Engine
{
	class CTexture;
	class CShader;
	class CUI_Animation;
	class CVIBuffer_Rect;
}

NS_BEGIN(Engine)

class ENGINE_DLL CUI_Image final : public CUIComponent
{
	friend  CUI_Animation;
public:
	typedef struct UI_Image_Desc : public CUIComponent::UICOMPONENT_DESC
	{
		uint32 iTextureIndex{0};
		std::wstring wstrFindTextureKey{};
	}UI_IMAGE_DESC;
protected:
	explicit		CUI_Image(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	explicit		CUI_Image(const CUI_Image& Proto);
	virtual			~CUI_Image();
private:
	HRESULT			Proto_Initialize();
	virtual HRESULT Clone_Initialize(void* pArg);
public:
	void			Set_TextureIndex(int32 iIndex) { m_iTextureIndex = iIndex; }
	void			Set_Aplha(float fAlpha) { m_fAlpha = fAlpha; }
	void			Set_PassName(const string& strPass) { m_strPass = strPass; }
public:
	CTexture*		Get_Texture() const					{ return m_pTexture;}
	uint32			Get_TexturCount() const;
	int32			Get_TextrueIndex() const			{ return m_iTextureIndex; }
	float			Get_Alpha()const					{ return m_fAlpha; }
private:
	HRESULT			Ready_Buffer();
	HRESULT			Ready_Resource(const wstring& wstrTextureKey);
public:
	virtual void    Pre_Update(float DT)	override;
	virtual void    Update(float DT)		override;
	virtual void    Late_Update(float DT)	override;
	virtual HRESULT Render(CCanvas* pCanvas)override;
private:
	/* Image 출력을 위한 도구 */
	CTexture*		m_pTexture;
	CVIBuffer_Rect* m_pBuffer;
	string			m_strPass;
	float			m_fAlpha;
	int32			m_iTextureIndex;
public:
	static  CUI_Image* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	CComponent* Clone(void* pArg) override;
	virtual void Free()override;
};

NS_END
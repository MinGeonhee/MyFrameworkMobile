#pragma once
#include "CBase.h"


namespace Engine
{
	class CShader;
	class CVIBuffer_Rect;
}

NS_BEGIN(Engine)

class CRenderTarget final : public CBase
{
private:
	explicit CRenderTarget(ComPtr<Device> pDevice,ComPtr<DeviceContext> pContext);
	virtual ~CRenderTarget() {}
public:
	HRESULT					Initialize(uint32 iWidth, uint32 iHeight,const Color& vColor, TEXTURE_FORMAT eFormat);
	HRESULT					Ready_Texture2D();
	HRESULT					Ready_ShaderResourceView();
	HRESULT					Ready_RenderTargetView();
	HRESULT					Clear_RenderTargetView();
public:
	HRESULT					Bind_ShaderResource(const string& strBindTexName,CShader* pShader);
public:
	ShaderResourceView*	Get_SRV()		const	{ return m_pSRV; }
	RenderTargetView*		Get_RTV()		const	{ return m_pRTV; }
	uint32						Get_Widht()		const	{ return m_iWidth; }
	uint32						Get_Height()	const	{ return m_iHeight; }
	const Color&				Get_ClearColor() const	{ return m_vClearColor; }
#ifdef  _DEBUG
	HRESULT					Ready_Debug(float fX , float fY , float fSizeX, float fSizeY);
	HRESULT					Render(CShader* pShader, CVIBuffer_Rect* pBuffer);
#endif //  _DEBUG
private:
	Color					m_vClearColor;
	uint32					m_iWidth;
	uint32					m_iHeight;
	TEXTURE_FORMAT				m_eFormat;
private:
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
	RenderTargetView* m_pRTV;
	ShaderResourceView*		m_pSRV;
	Texture2D*		m_pTexture2D;

#ifdef _DEBUG
	Float4x4				m_matWorld;
#endif

public:
	static CRenderTarget* Create(uint32 iWidth, uint32 iHeight, const Color& vColor, TEXTURE_FORMAT eFormat,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};

NS_END
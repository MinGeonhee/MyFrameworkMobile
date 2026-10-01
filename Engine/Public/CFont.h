#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class ENGINE_DLL CFont : public CBase
{
private:
	explicit CFont(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
	virtual ~CFont();

private:
	HRESULT Initialize(float fOriginFontSize,const wstring& wstrFontPath);
	/* 렌더하기위한 SpriteBatch 객체 , 랜더할 Text , 위치 , 색상 */
public:
	HRESULT Draw(SpriteBatch* pBatch , const TCHAR* pText , const Float2& vPosition , const fVector4_1& vColor , FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);
	HRESULT Draw(SpriteBatch* pBatch , const TCHAR* pText , const Float2& vPosition , const Float2& vSize , const fVector4_1& vColor, FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);
public:
	void					Set_FontOriginSize(float fOriginSize) { m_fOriginSize = fOriginSize; }
	float					Get_FontOriginSize() const			  { return m_fOriginSize; }
	
private:
	float					m_fOriginSize;
	SpriteFont*				m_pSpriteFont;
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pDeviceContext;
public:
	/* Font가 저장된 경로 */
	static CFont* Create( float fOriginSize ,const wstring& wstrFontPath ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};


NS_END

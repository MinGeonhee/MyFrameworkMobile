#pragma once
#include "CBase.h"

namespace Engine
{
	class CFont;
}


NS_BEGIN(Engine)

class CFontManager :public CBase
{
public:
	explicit CFontManager(ComPtr<Device>pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CFontManager() {}
private:
	HRESULT Initialize();
public:

	HRESULT Add_Font(float fOriginFontSize,const wstring& wstrAddKey,const wstring& wstrFontPath);
	CFont*  Get_Font(const wstring& wstrFindKey);
	HRESULT Draw(const wstring& wstringFindKey, const TCHAR* pText, const Float2& vPosition, const fVector4_1& vColor , FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);
	HRESULT Draw(const wstring& wstringFindKey,const TCHAR* pText, const Float2& vPosition, const Float2& vSize, const fVector4_1& vColor, FONT_RENDER_TYPE eType = FONT_RENDER_TYPE::DEFUALT);

private:
	unordered_map<wstring,CFont*>			m_umapFont;


	SpriteBatch*							m_pSpriteBatch;
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
public:
	static CFontManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual void Free() override;
};


NS_END
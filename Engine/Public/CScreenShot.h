#pragma once
#include "CBase.h"


NS_BEGIN(Engine)


class ENGINE_DLL CScreenShot final :    public CBase
{
private:
	explicit CScreenShot(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CScreenShot() {}
public:
	ComPtr<Texture2D> Get_ScreenShot_Textre2D();
	HRESULT					Save_ScreenShot(const wstring& wstrFilePath);
private:
	HRESULT	Initialize(ComPtr<Texture2D> pTexture2D);
private:
	ComPtr<Texture2D>				m_pTexture2D;
	ComPtr<Device>						m_pDevice;
	ComPtr<DeviceContext>				m_pDeviceContext;
public:
	static CScreenShot*					Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext, ComPtr<Texture2D> pTexture2D);
	virtual void						Free();
public:
	static uint32						m_iScreenShotNum;
};

NS_END
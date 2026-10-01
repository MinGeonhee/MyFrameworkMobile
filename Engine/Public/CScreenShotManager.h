#pragma once
#include "CBase.h"

namespace Engine
{
	class CScreenShot;
	class CQaudObject;
}

using ScreenShotFunc = std::function<HRESULT()>;
using ScreenShotOverLayFunc = std::function<HRESULT()>;

NS_BEGIN(Engine)

class CScreenShotManager :    public CBase
{
private:
	explicit CScreenShotManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual ~CScreenShotManager() {}

private:
	HRESULT								Initialize();
public:
	HRESULT								Add_ScreenShot(CONST wstring& strKey, CScreenShot* pScreenShot);
	CScreenShot*						Get_ScreenShot(CONST wstring& strKey);
	CScreenShot*						Get_OverLayScreenShot() { return m_pOverLayScreenShot; }
	/* 이건 안전버전 렌더가 다 완료된뒤 스크린샷 */
	void								Request_ScreenShot(bool isOverLay , _Outptr_  CScreenShot** ppScreenShot = NULLPTR);
	HRESULT								ExecutePending_ScreenShot();
	/* 바로요청은 위험 하지만 제공은함. */
	HRESULT								ScreenShot(bool isOverLay, _Outptr_ CScreenShot** ppScreenShot = NULLPTR);
private:
	CScreenShot*						Find_ScreenShot(CONST wstring& strKey);
private:
	FD	CScreenShot*					m_pOverLayScreenShot;
private:
	map<wstring, CScreenShot*>			m_mapScreenShot;
	queue<ScreenShotFunc>				m_queueScreenShotFunc;
private:
	ComPtr<Device>						m_pDevice;
	ComPtr<DeviceContext>				m_pDeviceContext;
	ComPtr<RTV>							m_pRTV;
public:
	static	CScreenShotManager* Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
	virtual	void				Free() override;
};

NS_END
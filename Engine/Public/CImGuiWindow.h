#pragma once

#include "CBase.h"
#ifdef _DEBUG
/* CImGuiBase를 통하여 원하는 Window Class를 생성 */
/* CImGuiManager에 등록하여 자동 Update 및 Render */



NS_BEGIN(Engine)

class ENGINE_DLL CImGuiWindow abstract : public CBase
{
	friend class CImGuiManager;  // Manager 전체 허용
	using Super = CImGuiWindow;

protected:
	explicit CImGuiWindow( HWND hWnd , ComPtr<Device> pDevice , ComPtr<DeviceContext> pDeviceContext);
	virtual	~CImGuiWindow() {};

protected:

	virtual	HRESULT Initialize();
	virtual	void	Update() PURE;

protected:
	HWND									m_hWnd;
	ComPtr<Device>							m_pDevice;
	ComPtr<DeviceContext>					m_pDeviceContext;
public:
	virtual void		Free() override;
};

NS_END;


#endif
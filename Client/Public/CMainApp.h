#pragma once

#include "CBase.h"

namespace Engine
{
	class CGameManager;
	class CImGuiManager;
}

/* 원본 Client CMainApp 구조 (Windows 실행부)                                    */
/* 실제 게임 초기화 / 업데이트 / 렌더는 Windows 와 Android 가 공유하는 Game 에 있다. */
class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT		Initialize(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI);
	void		Update(float DT);
	HRESULT		Render();
	void		Resize(uint32 iWinSizeX, uint32 iWinSizeY);

private:
	CGameManager*			m_pGameManager;
	ComPtr<Device>			m_pDevice;
	ComPtr<DeviceContext>	m_pContext;
#ifdef _DEBUG
	CImGuiManager*			m_pImGuiManager;
	bool					m_isOnImGui;
#endif

public:
	static CMainApp*	Create(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI);
	virtual void		Free() override;
};

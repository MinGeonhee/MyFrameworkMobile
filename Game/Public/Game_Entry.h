#pragma once

#include "Game_Define.h"

namespace Engine
{
	class CGameManager;
}

/* ------------------------------------------------------------------------------------------- */
/* Game 진입점 : 원본 Client CMainApp 의 Initialize / Update / Render 흐름을 그대로 담는다.          */
/* Windows Client(CMainApp) 와 Android(NativeMain) 가 이 함수들만 호출한다.                        */
/* ------------------------------------------------------------------------------------------- */
namespace Game
{
	/* 엔진 초기화 + 기본 리소스 + 셰이더 / 텍스처 / 폰트 / 렌더 상태 + 씬 등록 */
	HRESULT		Initialize(HWND hWnd, HINSTANCE hInst, uint32 iWinSizeX, uint32 iWinSizeY, GRAPHIC_API eGraphicAPI,
						   ComPtr<Device>* ppDevice, ComPtr<DeviceContext>* ppContext);

	/* 원본 Client 메인 루프 (60 FPS 제한) : 프레임을 실행했으면 true */
	bool		Frame();
	uint32		Get_FPS();

	void		Update(float fTimeDelta);
	HRESULT		Render();
	void		Release();

	/* 렌더 시작 / 끝 사이에 끼워 넣을 수 있는 콜백 (Windows Debug 의 ImGui 처리) */
	void		Set_RenderCallback(void(*pfnBeforeEndFrame)());
}

#pragma once
#include "CBase.h"

#if defined(_WIN32)
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#else
/* Android : DirectInput 이 없으므로 같은 모양의 마우스 상태 구조체를 터치로 채운다 */
struct DIMOUSESTATE
{
    LONG lX;
    LONG lY;
    LONG lZ;
    BYTE rgbButtons[4];
};
#endif

namespace Engine
{
    class CGameManager;
}


NS_BEGIN(Engine)

enum
{
    KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX + 1),
    KEY_STATE_COUNT = static_cast<int32>(KEY_STATE::END)
};

class CInputManager :     public CBase
{
private:
    explicit CInputManager(HINSTANCE hInst, HWND hwnd, ComPtr<Device> pDevice , ComPtr<DeviceContext> pDeviceContext ,uint32 iWinSizeX, uint32 iWinSizeY);
    virtual ~CInputManager() {}

public:

    HRESULT Initialize();
    void Update(float DT);

    // 누르고 있을 때
    bool GetButtonPress(KEY_TYPE key)
    {
        if (m_bKeyLock && key != KEY_TYPE::LBUTTON) return false;
        return GetState(key) == KEY_STATE::PRESS;
    }
    // 맨 처음 눌렀을 때
    bool GetButtonDown(KEY_TYPE key)
    {
        if (m_bKeyLock && key != KEY_TYPE::LBUTTON) return false;
        return GetState(key) == KEY_STATE::DOWN;
    }
    // 맨 처음 눌렀다 땠을 때
    bool GetButtonUp(KEY_TYPE key)
    {
        if (m_bKeyLock && key != KEY_TYPE::LBUTTON) return false;
        return GetState(key) == KEY_STATE::UP;
    }

    inline KEY_STATE GetState(KEY_TYPE key) { return m_vecStates[static_cast<uint8>(key)]; }

public:

    void Set_KeyLock(bool b) { m_bKeyLock = b; }
    bool Get_KeyLock() { return m_bKeyLock; }

public:

    // 현재 마우스의 특정 축 좌표를 반환
    long   Get_DIMouseMove(MOUSEMOVESTATE eMouseState) const
    {
        return *(((LONG*)&m_tMouseState) + eMouseState);
    }

    long   Get_DIMouseState(MOUSEKEYSTATE eMouse) const
    {
        return m_tMouseState.rgbButtons[eMouse];
    }

    const POINT& Get_MousePos() const  {
        int a = 0;
        return m_tMousePos;
    }
    const POINT& Get_PreFrameMousePos() const { return m_tPreFrameMoustPos; }
    void         Mouse_Fix();
    void         Mouse_Fix( uint32 iStartX , uint32 iStartY , uint32 iWinSizeX,uint32 iWinSizeY);
    void         Mouse_Set_Show(bool isShow);
    RAY_INFO     Get_LayWorld(const wstring& wstrCamName , UPDATE_MODE eType, RAY_TYPE eRayType);

#pragma region 모바일 입력 (Android 터치 / 가상 키)
public:
    /* 터치 → 마우스 (슬롯 0 : 왼쪽 버튼 , 화면 좌표는 SwapChain 픽셀 좌표) */
    void         Set_NativeTouch(uint32 iSlot, bool isPressed, float fScreenX, float fScreenY);
    /* 가상 패드 등에서 키 상태를 직접 넣는다 (다음 Update 에 반영) */
    void         Set_NativeKey(KEY_TYPE eKey, bool isPressed);
private:
    struct NATIVE_TOUCH
    {
        bool    isPressed{ false };
        float   fX{ 0.f }, fY{ 0.f };
    };
    std::mutex              m_NativeMutex;
    NATIVE_TOUCH            m_tNativeTouch[2]{};
    bool                    m_isNativeKey[KEY_TYPE_COUNT]{};
    POINT                   m_tPrevTouchPos{};
    bool                    m_isPrevTouch{ false };
#pragma endregion

private:
#if defined(_WIN32)
    LPDIRECTINPUT8			m_pInputSDK = nullptr;
#endif

private:
    HWND                    m_hWnd;
    HINSTANCE               m_hInstance;
    DIMOUSESTATE            m_tMouseState;
    POINT                   m_tMousePos;
    POINT                   m_tPreFrameMoustPos;

#if defined(_WIN32)
    LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;
#endif
    vector<KEY_STATE>       m_vecStates;

    uint32                  m_iWinSizeX;
    uint32                  m_iWinSizeY;


private:
    ComPtr<Device>          m_pDevice;
    ComPtr<DeviceContext>   m_pDeviceContext;
    CGameManager* m_pGameManager;
private:
    // 이벤트 발생했을 때 Key 잠금을 위해.

    bool                    m_bKeyLock = false;

public:
	static CInputManager*	Create(HINSTANCE hInst, HWND hwnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pDeviceContext,uint32 iWinSizeX, uint32 iWinSizeY);
	virtual	void			Free() override;
};

NS_END


#pragma once
#include "CBase.h"
#include "Engine_Struct.h"
class ENGINE_DLL CInputManager final : public CBase
{
private:CInputManager()=default;
public:
    ~CInputManager() override=default;
    void Update(float fTimeDelta);
    bool GetButtonPress(KEY_TYPE eKey) const;
    bool GetButtonDown(KEY_TYPE eKey) const;
    bool GetButtonUp(KEY_TYPE eKey) const;
    KEY_STATE GetState(KEY_TYPE eKey) const;
    void Set_KeyLock(bool isLock){m_isKeyLock=isLock;}
    bool Get_KeyLock()const{return m_isKeyLock;}
    // Android에서는 NativeActivity 입력 이벤트가 이 함수로 상태를 전달한다.
    void Set_NativeKeyState(uint32 iKeyCode,bool isPressed);
    void Set_NativeTouchState(uint32 iTouchIndex,bool isPressed,float fX,float fY);
    const Engine::TOUCH_STATE& GetTouchState(uint32 iTouchIndex)const;
    static CInputManager* Create(void* pNativeWindow);
    void Free() override;
private:
    void* m_pNativeWindow=nullptr;
    bool m_isKeyLock=false;
    std::array<bool,256> m_CurrentKey{};
    std::array<bool,256> m_PreviousKey{};
    std::array<KEY_STATE,256> m_KeyState{};
    std::array<Engine::TOUCH_STATE,2> m_TouchState{};
    std::array<bool,2> m_PreviousTouchPressed{};
    int32 m_iMouseTouchIndex=-1;
};

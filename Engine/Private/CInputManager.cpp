#include "pch.h"
#include "CInputManager.h"

CInputManager* CInputManager::Create(void* pNativeWindow){auto*p=new CInputManager{};p->m_pNativeWindow=pNativeWindow;p->m_KeyState.fill(KEY_STATE::NONE);return p;}

void CInputManager::Update(float)
{
#if defined(_WIN32)
    for(uint32 i=0;i<m_CurrentKey.size();++i)m_CurrentKey[i]=(GetAsyncKeyState(static_cast<int>(i))&0x8000)!=0;

    const bool isMousePressed=m_CurrentKey[static_cast<uint32>(KEY_TYPE::LBUTTON)];
    HWND hWnd=static_cast<HWND>(m_pNativeWindow);
    if(hWnd!=nullptr)
    {
        POINT Point{};GetCursorPos(&Point);ScreenToClient(hWnd,&Point);
        RECT ClientRect{};GetClientRect(hWnd,&ClientRect);
        if(isMousePressed&&m_iMouseTouchIndex<0)m_iMouseTouchIndex=Point.x<(ClientRect.right-ClientRect.left)/2?0:1;
        if(m_iMouseTouchIndex>=0)Set_NativeTouchState(static_cast<uint32>(m_iMouseTouchIndex),isMousePressed,static_cast<float>(Point.x),static_cast<float>(Point.y));
        if(!isMousePressed)m_iMouseTouchIndex=-1;
    }
#endif

    for(size_t i=0;i<m_CurrentKey.size();++i){if(m_CurrentKey[i])m_KeyState[i]=m_PreviousKey[i]?KEY_STATE::PRESS:KEY_STATE::DOWN;else m_KeyState[i]=m_PreviousKey[i]?KEY_STATE::UP:KEY_STATE::NONE;m_PreviousKey[i]=m_CurrentKey[i];}
    for(size_t i=0;i<m_TouchState.size();++i)
    {
        auto&t=m_TouchState[i];const bool was=m_PreviousTouchPressed[i];t.isDown=t.isPressed&&!was;t.isUp=!t.isPressed&&was;
        if(t.isDown){t.fStartX=t.fX;t.fStartY=t.fY;t.fDeltaX=t.fDeltaY=0.f;}
        m_PreviousTouchPressed[i]=t.isPressed;
    }
}

KEY_STATE CInputManager::GetState(KEY_TYPE eKey)const{const size_t i=static_cast<size_t>(eKey);return i<m_KeyState.size()?m_KeyState[i]:KEY_STATE::NONE;}
bool CInputManager::GetButtonPress(KEY_TYPE eKey)const{return !m_isKeyLock&&GetState(eKey)==KEY_STATE::PRESS;}
bool CInputManager::GetButtonDown(KEY_TYPE eKey)const{return !m_isKeyLock&&GetState(eKey)==KEY_STATE::DOWN;}
bool CInputManager::GetButtonUp(KEY_TYPE eKey)const{return !m_isKeyLock&&GetState(eKey)==KEY_STATE::UP;}
void CInputManager::Set_NativeKeyState(uint32 iKeyCode,bool isPressed){if(iKeyCode<m_CurrentKey.size())m_CurrentKey[iKeyCode]=isPressed;}
void CInputManager::Set_NativeTouchState(uint32 i,bool pressed,float x,float y){if(i>=m_TouchState.size())return;auto&t=m_TouchState[i];t.fDeltaX=pressed&&t.isPressed?x-t.fX:0.f;t.fDeltaY=pressed&&t.isPressed?y-t.fY:0.f;t.fX=x;t.fY=y;t.isPressed=pressed;}
const Engine::TOUCH_STATE& CInputManager::GetTouchState(uint32 i)const{static const Engine::TOUCH_STATE Empty{};return i<m_TouchState.size()?m_TouchState[i]:Empty;}
void CInputManager::Free(){m_pNativeWindow=nullptr;m_CurrentKey.fill(false);m_PreviousKey.fill(false);m_KeyState.fill(KEY_STATE::NONE);m_TouchState={};m_PreviousTouchPressed.fill(false);m_iMouseTouchIndex=-1;CBase::Free();}

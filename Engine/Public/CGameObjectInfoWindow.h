#pragma once
#ifdef _DEBUG

#include "CImGuiWindow.h"

namespace Engine
{
    class CGameObject;
}

NS_BEGIN(Engine)

class ENGINE_DLL CGameObjectInfoWindow  final:    public CImGuiWindow
{
private:
    explicit                CGameObjectInfoWindow(HWND hWnd,ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
    virtual                 ~CGameObjectInfoWindow() {}
public:
    virtual void            Update() override;
private:
    void			        MainWindow();
    void                    GameObjectInfoWindow();
public:
    void                    Add_GameObject(CGameObject* pGameObject);
private:

    list<CGameObject*>      m_listTargetGameObject;

public:
    static  CGameObjectInfoWindow* Create(HWND hWnd , ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual void Free() override;
};


NS_END
#endif
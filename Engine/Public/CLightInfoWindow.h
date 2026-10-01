#pragma once

#ifdef _DEBUG

#include "CImGuiWindow.h"

namespace Engine
{
    class   CLightManager;
    class   CLight;
    class   CGameManager;
}

NS_BEGIN(Engine)


class ENGINE_DLL CLightInfoWindow : public CImGuiWindow
{
private:
    explicit             CLightInfoWindow(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual               ~CLightInfoWindow() {}
public:
    virtual void            Update() override;
private:
    void			       MainWindow();
    void                  LightListWindow();
    void                  LightInfoWindow();

private:
    CLight*           m_pTargetLight;
    CGameManager* m_pGameManager;
public:
    static  CLightInfoWindow* Create(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual void            Free() override;
};

NS_END


#endif
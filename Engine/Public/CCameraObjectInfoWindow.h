#pragma once
#include "CImGuiWindow.h"

#ifdef _DEBUG

namespace Engine
{
    class CCameraObjectBase;
    class CUICameraObject;
    class CDefaultCameraObject;
}


NS_BEGIN(Engine)

class ENGINE_DLL CCameraObjectInfoWindow final : public CImGuiWindow
{
    friend CCameraObjectBase;
    friend CDefaultCameraObject;
private:
    explicit CCameraObjectInfoWindow(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual ~CCameraObjectInfoWindow() {}
public:
    virtual void            Update() override;
private:
    void			                MainWindow();
    void                            CameraObjectInfoWindow();

public:
    void                            SetTargetGameObject(CCameraObjectBase* pTargetGameObject) { m_pTargetCameraObject = pTargetGameObject; }

    void                            SetDefault(const Float3& tOffset, float fNear, float fFar, float fFov) { m_tDefautlOffset = tOffset; m_fDefaultNear = fNear;  m_fDefaultFov = fFov;  m_fDefaultFar = fFar; }
    void                            SetDefaultOffset(const Float3& tOffset) { m_tDefautlOffset = tOffset; }

private:

    CCameraObjectBase*              m_pTargetCameraObject;
    vector<CCameraObjectBase*>      m_vecTargetCameraObject;

    Float3                          m_tDefautlOffset;
    float                           m_fDefaultNear;
    float                           m_fDefaultFar;
    float                           m_fDefaultFov;

public:
    static  CCameraObjectInfoWindow* Create(HWND hWnd, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext);
    virtual void Free()
    {
            __super::Free();

            /* CCameraObjectInfoWindow::Free() */

            m_pTargetCameraObject = nullptr;
            m_vecTargetCameraObject.clear();
    }
};

NS_END

#endif // DEBUG


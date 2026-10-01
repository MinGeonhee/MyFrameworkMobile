#pragma once
#include "CBase.h"

NS_BEGIN(Engine)

class CGameObject;

class ENGINE_DLL CComponent abstract :
    public CBase 
{
protected:
    explicit CComponent(ComPtr<Device> pDevice , ComPtr<DeviceContext> pContext);
    explicit CComponent(const CComponent& Prototype);
    virtual ~CComponent();
public:
    void                    Set_Owner(CBase* pOwner);
    CBase*                  Get_Owner() const { return m_pOwner; }
protected:
    virtual HRESULT         Proto_Initialize();
    virtual HRESULT         Clone_Initialize(void* pArg);
public:
    virtual void            Pre_Update(float DT);
    virtual void            Update(float DT);
    virtual void            Late_Update(float DT);
    virtual HRESULT         Render(FD CCameraObjectBase* pRenderCam);
public:
    bool                    Get_IsActive() { return m_isActive; }
    void                    Set_IsActive(bool isActive) { m_isActive = isActive; }
#ifdef _DEBUG
    virtual void            ImGuiUpdate();
#endif // _DEBUG
protected:
    bool                    m_isActive;
    ComPtr<Device>          m_pDevice;
    ComPtr<DeviceContext>   m_pDeviceContext;
protected:
     CBase*                 m_pOwner;
     MAKE_TYPE              m_eMakeType;
public:
    virtual CComponent* Clone( void* pArg) PURE;
    virtual void Free()override;
};

NS_END

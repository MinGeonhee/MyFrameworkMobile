#pragma once
#include "CBase.h"

class ENGINE_DLL CComponent abstract : public CBase
{
protected:
    explicit CComponent(Device* pDevice, Context* pDeviceContext);
    explicit CComponent(const CComponent& Prototype);
    ~CComponent() override = default;

public:
    void Set_Owner(CBase* pOwner);
    CBase* Get_Owner() const { return m_pOwner; }

    virtual HRESULT Proto_Initialize();
    virtual HRESULT Clone_Initialize(void* pArg);
    virtual void Pre_Update(float fTimeDelta);
    virtual void Update(float fTimeDelta);
    virtual void Late_Update(float fTimeDelta);
    virtual HRESULT Render();

    bool Get_IsActive() const { return m_isActive; }
    void Set_IsActive(bool isActive) { m_isActive = isActive; }

protected:
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
    CBase* m_pOwner = nullptr;
    bool m_isActive = true;

public:
    virtual CComponent* Clone(void* pArg) = 0;
    void Free() override;
};

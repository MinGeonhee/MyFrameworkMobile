#pragma once
#include "CComponent.h"
#include "Engine_Struct.h"

class CGameObject;

class ENGINE_DLL CCollider final : public CComponent
{
private:
    explicit CCollider(Device* pDevice, Context* pDeviceContext);
    explicit CCollider(const CCollider& Prototype);
    ~CCollider() override = default;

public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    bool Intersects(const CCollider* pOther) const;
    const Engine::COLLIDER_DESC& Get_Desc() const { return m_Desc; }
    void Set_Center(float x, float y, float z);
    CGameObject* Get_OwnerGameObject() const;
    static CCollider* Create(Device* pDevice, Context* pDeviceContext, const Engine::COLLIDER_DESC& Desc);
    CComponent* Clone(void* pArg) override;
    void Free() override;

private:
    Engine::COLLIDER_DESC m_Desc{};
};

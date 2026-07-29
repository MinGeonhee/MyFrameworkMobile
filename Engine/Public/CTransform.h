#pragma once
#include "CComponent.h"
#include "Engine_Struct.h"
#include "Common/interface/BasicMath.hpp"

class ENGINE_DLL CTransform final : public CComponent
{
private:
    explicit CTransform(Device* pDevice, Context* pContext);
    explicit CTransform(const CTransform& Prototype);
    ~CTransform() override = default;
public:
    HRESULT Proto_Initialize() override;
    HRESULT Clone_Initialize(void* pArg) override;
    void Set_Position(const Diligent::float3& Position);
    void Set_Rotation(const Diligent::float3& RotationRadian);
    void Add_Rotation(float fPitch, float fYaw, float fRoll = 0.f);
    void Move_Local(float fRight, float fUp, float fForward, float fTimeDelta);
    void Set_SpeedPerSecond(float fSpeed) { m_fSpeedPerSecond=fSpeed; }
    const Diligent::float3& Get_Position() const { return m_Position; }
    const Diligent::float3& Get_Rotation() const { return m_Rotation; }
    const Diligent::float4x4& Get_WorldMatrix() const { return m_WorldMatrix; }
    Diligent::float4x4 Get_ViewMatrix() const { return m_WorldMatrix.Inverse(); }
    Diligent::float3 Get_Right() const;
    Diligent::float3 Get_Up() const;
    Diligent::float3 Get_Look() const;
    static CTransform* Create(Device* pDevice, Context* pContext);
    CComponent* Clone(void* pArg) override;
    void Free() override;
private:
    void Update_WorldMatrix();
    Diligent::float3 m_Position{0,0,0},m_Rotation{0,0,0},m_Scale{1,1,1};
    Diligent::float4x4 m_WorldMatrix=Diligent::float4x4::Identity();
    float m_fSpeedPerSecond=5.f,m_fRotationPerSecond=1.5f;
};

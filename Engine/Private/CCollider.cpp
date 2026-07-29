#include "pch.h"
#include "CCollider.h"
#include "CGameObject.h"
#include <cmath>

CCollider::CCollider(Device* pDevice, Context* pDeviceContext) : CComponent{pDevice, pDeviceContext} {}
CCollider::CCollider(const CCollider& Prototype) : CComponent{Prototype}, m_Desc{Prototype.m_Desc} {}
HRESULT CCollider::Proto_Initialize() { return CComponent::Proto_Initialize(); }
HRESULT CCollider::Clone_Initialize(void* pArg)
{
    if (FAILED(CComponent::Clone_Initialize(pArg))) return E_FAIL;
    if (pArg != nullptr) m_Desc = *static_cast<Engine::COLLIDER_DESC*>(pArg);
    return S_OK;
}
void CCollider::Set_Center(float x, float y, float z)
{ m_Desc.vCenter[0] = x; m_Desc.vCenter[1] = y; m_Desc.vCenter[2] = z; }
CGameObject* CCollider::Get_OwnerGameObject() const { return static_cast<CGameObject*>(m_pOwner); }

bool CCollider::Intersects(const CCollider* pOther) const
{
    if (pOther == nullptr || !m_Desc.isActive || !pOther->m_Desc.isActive) return false;
    if (m_Desc.eType == COLLIDER_TYPE::SPHERE && pOther->m_Desc.eType == COLLIDER_TYPE::SPHERE)
    {
        const float dx = m_Desc.vCenter[0] - pOther->m_Desc.vCenter[0];
        const float dy = m_Desc.vCenter[1] - pOther->m_Desc.vCenter[1];
        const float dz = m_Desc.vCenter[2] - pOther->m_Desc.vCenter[2];
        const float radius = m_Desc.fRadius + pOther->m_Desc.fRadius;
        return dx * dx + dy * dy + dz * dz <= radius * radius;
    }
    // 초기 이식 단계의 Sphere/AABB 혼합은 각 축 범위로 검사한다.
    for (uint32 i = 0; i < 3; ++i)
    {
        const float lhs = m_Desc.eType == COLLIDER_TYPE::SPHERE ? m_Desc.fRadius : m_Desc.vExtents[i];
        const float rhs = pOther->m_Desc.eType == COLLIDER_TYPE::SPHERE ? pOther->m_Desc.fRadius : pOther->m_Desc.vExtents[i];
        if (std::abs(m_Desc.vCenter[i] - pOther->m_Desc.vCenter[i]) > lhs + rhs) return false;
    }
    return true;
}
CCollider* CCollider::Create(Device* pDevice, Context* pDeviceContext, const Engine::COLLIDER_DESC& Desc)
{
    CCollider* pInstance = new CCollider{pDevice, pDeviceContext};
    if (pInstance == nullptr || FAILED(pInstance->Proto_Initialize())) { if (pInstance) pInstance->Release(); return nullptr; }
    pInstance->m_Desc = Desc;
    return pInstance;
}
CComponent* CCollider::Clone(void* pArg)
{
    CCollider* pInstance = new CCollider{*this};
    if (pInstance == nullptr || FAILED(pInstance->Clone_Initialize(pArg))) { if (pInstance) pInstance->Release(); return nullptr; }
    return pInstance;
}
void CCollider::Free() { CComponent::Free(); }

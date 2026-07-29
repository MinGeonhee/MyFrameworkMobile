#include "pch.h"
#include "CCollisionManager.h"
#include "CCollider.h"
#include "CGameObject.h"

CCollisionManager::CCollisionManager(uint32 iMaxColliderGroupCount) : m_ColliderGroups(iMaxColliderGroupCount) {}

HRESULT CCollisionManager::Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID)
{
    if (iSrcGroupID >= m_ColliderGroups.size() || iDstGroupID >= m_ColliderGroups.size()) return E_INVALIDARG;
    const auto Table = std::make_pair(iSrcGroupID, iDstGroupID);
    if (std::find(m_CollisionTables.begin(), m_CollisionTables.end(), Table) == m_CollisionTables.end())
        m_CollisionTables.push_back(Table);
    return S_OK;
}

HRESULT CCollisionManager::Add_Collider(uint32 iGroupID, CCollider* pCollider)
{
    if (iGroupID >= m_ColliderGroups.size() || pCollider == nullptr) return E_INVALIDARG;
    auto& Group = m_ColliderGroups[iGroupID];
    if (std::find(Group.begin(), Group.end(), pCollider) != Group.end()) return S_FALSE;
    pCollider->AddRef();
    Group.push_back(pCollider);
    return S_OK;
}

void CCollisionManager::Update(float)
{
    std::set<CollisionPair> Current;
    for (const auto& Table : m_CollisionTables)
    {
        const auto& Src = m_ColliderGroups[Table.first];
        const auto& Dst = m_ColliderGroups[Table.second];
        for (CCollider* pSrc : Src) for (CCollider* pDst : Dst)
        {
            if (pSrc == nullptr || pDst == nullptr || pSrc == pDst || !pSrc->Intersects(pDst)) continue;
            CollisionPair Pair = pSrc < pDst ? CollisionPair{pSrc, pDst} : CollisionPair{pDst, pSrc};
            if (!Current.insert(Pair).second) continue;
            CGameObject* pSrcOwner = pSrc->Get_OwnerGameObject();
            CGameObject* pDstOwner = pDst->Get_OwnerGameObject();
            const bool isEnter = m_PreviousCollisions.find(Pair) == m_PreviousCollisions.end();
            if (pSrcOwner) (isEnter ? pSrcOwner->OnCollisionEnter(pDstOwner, pDst) : pSrcOwner->OnCollision(pDstOwner, pDst));
            if (pDstOwner) (isEnter ? pDstOwner->OnCollisionEnter(pSrcOwner, pSrc) : pDstOwner->OnCollision(pSrcOwner, pSrc));
        }
    }
    for (const CollisionPair& Pair : m_PreviousCollisions) if (Current.find(Pair) == Current.end())
    {
        CGameObject* pFirst = Pair.first->Get_OwnerGameObject();
        CGameObject* pSecond = Pair.second->Get_OwnerGameObject();
        if (pFirst) pFirst->OnCollisionExit(pSecond, Pair.second);
        if (pSecond) pSecond->OnCollisionExit(pFirst, Pair.first);
    }
    m_PreviousCollisions.swap(Current);
}

void CCollisionManager::Clean_DeadCollision()
{
    for (auto& Group : m_ColliderGroups)
    {
        auto Iter = std::remove_if(Group.begin(), Group.end(), [](CCollider* pCollider)
        {
            CGameObject* pOwner = pCollider != nullptr ? pCollider->Get_OwnerGameObject() : nullptr;
            if (pCollider == nullptr || (pOwner != nullptr && pOwner->Get_Dead())) { if (pCollider) pCollider->Release(); return true; }
            return false;
        });
        Group.erase(Iter, Group.end());
    }
}

void CCollisionManager::Clear_Collision()
{
    for (auto& Group : m_ColliderGroups) { for (CCollider* pCollider : Group) if (pCollider) pCollider->Release(); Group.clear(); }
    m_PreviousCollisions.clear();
}
CCollisionManager* CCollisionManager::Create(uint32 iMaxColliderGroupCount) { return iMaxColliderGroupCount ? new CCollisionManager{iMaxColliderGroupCount} : nullptr; }
void CCollisionManager::Free() { Clear_Collision(); m_CollisionTables.clear(); CBase::Free(); }

#pragma once
#include "CBase.h"

#include <set>
#include <utility>
#include <vector>

class CCollider;

class ENGINE_DLL CCollisionManager final : public CBase
{
private:
    explicit CCollisionManager(uint32 iMaxColliderGroupCount);
    ~CCollisionManager() override = default;

public:
    HRESULT Register_CollisionTable(uint32 iSrcGroupID, uint32 iDstGroupID);
    HRESULT Add_Collider(uint32 iGroupID, CCollider* pCollider);
    void Update(float fTimeDelta);
    void Clear_Collision();
    void Clean_DeadCollision();
    static CCollisionManager* Create(uint32 iMaxColliderGroupCount = 32);
    void Free() override;

private:
    using CollisionPair = std::pair<CCollider*, CCollider*>;
    std::vector<std::pair<uint32, uint32>> m_CollisionTables;
    std::vector<std::vector<CCollider*>> m_ColliderGroups;
    std::set<CollisionPair> m_PreviousCollisions;
};


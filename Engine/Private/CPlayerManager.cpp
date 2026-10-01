#include "CPlayerManager.h"
#include "CGameObject.h"
#include "CTransform.h"


CPlayerManager::CPlayerManager()
    : m_umapPlayer{}
{
}

HRESULT CPlayerManager::Add_Player(const wstring& wstrAddKey, CGameObject* pPlayer)
{
    auto pFind = m_umapPlayer.find(wstrAddKey);
    
    if (pFind != m_umapPlayer.end())
        return E_FAIL;

    m_umapPlayer.emplace(wstrAddKey,pPlayer);

    Safe_AddRef(pPlayer);

    return S_OK;
}

CGameObject* CPlayerManager::Get_Player(const wstring& wstrFindKey)
{
    auto pFind = m_umapPlayer.find(wstrFindKey);

    if (pFind == m_umapPlayer.end())
        return nullptr;

    return pFind->second;
}

void CPlayerManager::Clear_Player()
{
    for (auto& Player : m_umapPlayer)
        Safe_Release(Player.second);

    return;
}


CGameObject* CPlayerManager::Compute_CheckRange(const fVector4_1& vCheckPos , float fCheckRange , CGameObject* pCurTargetPlayer)
{
    float        fRange = fCheckRange * fCheckRange;

    /* 이미 정해진 타겟이 있다면 */
    if (pCurTargetPlayer)
    {
        float Length = XMVectorGetX(XMVector3LengthSq((pCurTargetPlayer->GetTransform()->Get_State_ToCal(Engine::STATE::POSITION) - vCheckPos)));

        if (fRange > Length)
            return pCurTargetPlayer;
    }


    CGameObject* pNearPlayer = nullptr;
    float        MinDistance = 9999999.f;
    
    for (auto& Player : m_umapPlayer)
    {
        CGameObject* pCheckPlayer = Player.second;

        float Length = XMVectorGetX(XMVector3LengthSq((pCheckPlayer->GetTransform()->Get_State_ToCal(Engine::STATE::POSITION) - vCheckPos)));

        if ( Length < MinDistance )
        {
            MinDistance = Length;
            pNearPlayer = pCheckPlayer;
        }
    }

    return fRange > MinDistance ? pNearPlayer : nullptr;
}

CPlayerManager* CPlayerManager::Create()
{
    return new CPlayerManager;
}

void CPlayerManager::Free()
{
    __super::Free();


    for (auto& Player : m_umapPlayer)
        Safe_Release(Player.second);
}

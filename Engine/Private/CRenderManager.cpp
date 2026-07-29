#include "pch.h"
#include "CRenderManager.h"
#include "CGameObject.h"

CRenderManager* CRenderManager::Create() { return new CRenderManager{}; }
HRESULT CRenderManager::Ready_Default() { return S_OK; }

HRESULT CRenderManager::Add_RenderGroup(RENDER_GROUP eRenderGroup, CGameObject* pGameObject)
{
    const size_t iGroup = static_cast<size_t>(eRenderGroup);
    if (iGroup >= m_RenderGroups.size() || pGameObject == nullptr) return E_INVALIDARG;
    // RenderGroup은 프레임 동안만 참조하므로 소유권을 갖지 않는다.
    m_RenderGroups[iGroup].push_back(pGameObject);
    return S_OK;
}

HRESULT CRenderManager::RenderGroup()
{
    constexpr RENDER_GROUP Order[] = {
        RENDER_GROUP::SKYBOX, RENDER_GROUP::PRIORITY, RENDER_GROUP::NONBLEND,
        RENDER_GROUP::NONLIGHT, RENDER_GROUP::BLEND};
    for (RENDER_GROUP eGroup : Order)
        for (CGameObject* pObject : m_RenderGroups[static_cast<size_t>(eGroup)])
            if (pObject != nullptr && !pObject->Get_Dead() &&
                pObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render) && FAILED(pObject->Render()))
                return E_FAIL;
    return S_OK;
}

HRESULT CRenderManager::Clean_RenderGroup()
{
    for (auto& Group : m_RenderGroups) Group.clear();
    return S_OK;
}

size_t CRenderManager::Get_RenderGroupSize(RENDER_GROUP eRenderGroup) const
{
    const size_t iGroup = static_cast<size_t>(eRenderGroup);
    return iGroup < m_RenderGroups.size() ? m_RenderGroups[iGroup].size() : 0;
}

void CRenderManager::Free()
{
    Clean_RenderGroup();
    CBase::Free();
}

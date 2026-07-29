#include "pch.h"
#include "CLayer.h"
#include "CGameObject.h"

HRESULT CLayer::Initialize()
{
    m_vecGameObject.reserve(100);
    return S_OK;
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
    if (pGameObject == nullptr)
        return E_INVALIDARG;
    // Clone 결과의 최초 소유권을 Layer가 그대로 넘겨받는다.
    m_vecGameObject.push_back(pGameObject);
    return S_OK;
}

CGameObject* CLayer::Find_GameObject(CGameObject* pGameObject) const
{
    const auto Iter = std::find(m_vecGameObject.begin(), m_vecGameObject.end(), pGameObject);
    return Iter != m_vecGameObject.end() ? *Iter : nullptr;
}

CGameObject* CLayer::Find_GameObject(const std::wstring& strGameObjectName) const
{
    const auto Iter = std::find_if(m_vecGameObject.begin(), m_vecGameObject.end(), [&strGameObjectName](CGameObject* pObject) {
        return pObject != nullptr && pObject->GetName() == strGameObjectName;
    });
    return Iter != m_vecGameObject.end() ? *Iter : nullptr;
}

void CLayer::Pre_Update(float fTimeDelta)
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr && pObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::PreUpdate))
            pObject->Pre_Update(fTimeDelta);
}

void CLayer::Update(float fTimeDelta)
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr && pObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Update))
            pObject->Update(fTimeDelta);
}

void CLayer::Late_Update(float fTimeDelta)
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr && pObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::LateUpdate))
            pObject->Late_Update(fTimeDelta);
}

HRESULT CLayer::Add_RenderGroup()
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr && pObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Add_RenderGroup) &&
            FAILED(pObject->Add_RenderGroup()))
            return E_FAIL;
    return S_OK;
}

void CLayer::Add_ColliderGroup()
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr && !pObject->Get_Dead())
            pObject->Add_ColliderGroup();
}

void CLayer::Clean_DeadObject()
{
    const auto Iter = std::remove_if(m_vecGameObject.begin(), m_vecGameObject.end(), [](CGameObject* pObject) {
        if (pObject == nullptr)
            return true;
        if (!pObject->Get_Dead())
            return false;
        pObject->Release();
        return true;
    });
    m_vecGameObject.erase(Iter, m_vecGameObject.end());
}

void CLayer::Clear_GameObject()
{
    for (CGameObject* pObject : m_vecGameObject)
        if (pObject != nullptr)
            pObject->Release();
    m_vecGameObject.clear();
}

CLayer* CLayer::Create()
{
    CLayer* pInstance = new CLayer{};
    if (FAILED(pInstance->Initialize())) { pInstance->Release(); return nullptr; }
    return pInstance;
}

void CLayer::Free()
{
    Clear_GameObject();
    CBase::Free();
}

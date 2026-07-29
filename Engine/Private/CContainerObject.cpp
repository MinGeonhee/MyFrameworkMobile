#include "pch.h"
#include "CContainerObject.h"
#include "CPartObject.h"
#include "CGameManager.h"

CContainerObject::CContainerObject(Device* pDevice, Context* pDeviceContext)
    : CGameObject{pDevice, pDeviceContext}
{
}
CContainerObject::CContainerObject(const CContainerObject& Prototype) : CGameObject{Prototype} {}
HRESULT CContainerObject::Proto_Initialize() { return CGameObject::Proto_Initialize(); }
HRESULT CContainerObject::Clone_Initialize(void* pArg)
{
    if (FAILED(CGameObject::Clone_Initialize(pArg))) return E_FAIL;
    return Ready_PartObject();
}
HRESULT CContainerObject::Ready_Component(void* pArg) { return CGameObject::Ready_Component(pArg); }
HRESULT CContainerObject::Ready_PartObject() { return S_OK; }

void CContainerObject::Pre_Update(float fTimeDelta)
{
    CGameObject::Pre_Update(fTimeDelta);
    for (auto& Pair : m_mapPartObject) if (Pair.second != nullptr) Pair.second->Pre_Update(fTimeDelta);
}
void CContainerObject::Update(float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    for (auto& Pair : m_mapPartObject) if (Pair.second != nullptr) Pair.second->Update(fTimeDelta);
}
void CContainerObject::Late_Update(float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);
    for (auto& Pair : m_mapPartObject) if (Pair.second != nullptr) Pair.second->Late_Update(fTimeDelta);
}
HRESULT CContainerObject::Add_RenderGroup()
{
    for (auto& Pair : m_mapPartObject)
        if (Pair.second != nullptr && FAILED(Pair.second->Add_RenderGroup())) return E_FAIL;
    return CGameObject::Add_RenderGroup();
}
HRESULT CContainerObject::Render()
{
    if (FAILED(CGameObject::Render())) return E_FAIL;
    for (auto& Pair : m_mapPartObject)
        if (Pair.second != nullptr && FAILED(Pair.second->Render())) return E_FAIL;
    return S_OK;
}
void CContainerObject::Set_Dead()
{
    CGameObject::Set_Dead();
    for (auto& Pair : m_mapPartObject) if (Pair.second != nullptr) Pair.second->Set_Dead();
}

HRESULT CContainerObject::Add_PartObject(uint32 iFindLevelID,
                                         const std::wstring& strPrototypeTag,
                                         const std::wstring& strPartTag,
                                         void* pArg,
                                         CPartObject** ppPartObject)
{
    if (strPartTag.empty() || Find_PartObject(strPartTag) != nullptr || m_pGameManager == nullptr)
        return E_INVALIDARG;
    CPartObject* pPart = static_cast<CPartObject*>(m_pGameManager->PrototypeManager_Clone_Prototype(
        PROTOTYPE_TYPE::GAMEOBJECT, iFindLevelID, strPrototypeTag, pArg));
    if (pPart == nullptr) return E_FAIL;
    m_mapPartObject.emplace(strPartTag, pPart);
    if (ppPartObject != nullptr) { *ppPartObject = pPart; pPart->AddRef(); }
    return S_OK;
}

CPartObject* CContainerObject::Find_PartObject(const std::wstring& strPartTag) const
{
    const auto Iter = m_mapPartObject.find(strPartTag);
    return Iter != m_mapPartObject.end() ? Iter->second : nullptr;
}

void CContainerObject::Free()
{
    for (auto& Pair : m_mapPartObject) if (Pair.second != nullptr) Pair.second->Release();
    m_mapPartObject.clear();
    CGameObject::Free();
}

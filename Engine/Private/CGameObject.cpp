#include "pch.h"
#include "CGameObject.h"

#include "CComponent.h"
#include "CGameManager.h"

CGameObject::CGameObject(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
    , m_pGameManager{CGameManager::GetInstance()}
{
    m_isUpdatePhaseActive.fill(true);
    if (m_pGameManager != nullptr)
        m_pGameManager->AddRef();
}

CGameObject::CGameObject(const CGameObject& Prototype)
    : CBase{Prototype}
    , m_pDevice{Prototype.m_pDevice}
    , m_pDeviceContext{Prototype.m_pDeviceContext}
    , m_pGameManager{Prototype.m_pGameManager}
    , m_strName{Prototype.m_strName}
    , m_isUpdatePhaseActive{Prototype.m_isUpdatePhaseActive}
    , m_isDead{Prototype.m_isDead}
    , m_isExpectedDelete{Prototype.m_isExpectedDelete}
    , m_isPartObject{Prototype.m_isPartObject}
    , m_iIndex{Prototype.m_iIndex}
    , m_iEventIndex{Prototype.m_iEventIndex}
    , m_eMakeType{MAKE_TYPE::CLONE}
{
    if (m_pGameManager != nullptr)
        m_pGameManager->AddRef();
}

HRESULT CGameObject::Proto_Initialize()
{
    return S_OK;
}

HRESULT CGameObject::Clone_Initialize(void* pArg)
{
    if (pArg != nullptr)
    {
        const Engine::GAMEOBJECT_DESC* pDesc = static_cast<const Engine::GAMEOBJECT_DESC*>(pArg);
        m_strName = pDesc->strGameObjectName;
        m_iEventIndex = pDesc->iEventIndex;
    }
    return Ready_Component(pArg);
}

HRESULT CGameObject::Ready_Component(void*)
{
    return S_OK;
}

HRESULT CGameObject::Add_Component(uint32 iFindLevelID,
                                   const std::wstring& strPrototypeTag,
                                   const std::wstring& strComponentTag,
                                   void* pArg,
                                   CComponent** ppOut)
{
    if (m_pGameManager == nullptr || strComponentTag.empty() || Find_Component(strComponentTag) != nullptr)
        return E_FAIL;

    CComponent* pComponent = static_cast<CComponent*>(
        m_pGameManager->PrototypeManager_Clone_Prototype(
            PROTOTYPE_TYPE::COMPONENT,
            iFindLevelID,
            strPrototypeTag,
            pArg));

    if (pComponent == nullptr)
        return E_FAIL;

    pComponent->Set_Owner(this);
    m_mapComponent.emplace(strComponentTag, pComponent);

    if (ppOut != nullptr)
    {
        *ppOut = pComponent;
        pComponent->AddRef();
    }

    return S_OK;
}

CComponent* CGameObject::Find_Component(const std::wstring& strComponentTag) const
{
    const auto Iter = m_mapComponent.find(strComponentTag);
    return Iter != m_mapComponent.end() ? Iter->second : nullptr;
}

void CGameObject::Pre_Update(float)
{
}

void CGameObject::Update(float)
{
}

void CGameObject::Late_Update(float)
{
}

HRESULT CGameObject::Render()
{
    return S_OK;
}

HRESULT CGameObject::Add_RenderGroup()
{
    return S_OK;
}

void CGameObject::Add_ColliderGroup()
{
}

void CGameObject::OnCollisionEnter(CGameObject*, CCollider*) {}
void CGameObject::OnCollision(CGameObject*, CCollider*) {}
void CGameObject::OnCollisionExit(CGameObject*, CCollider*) {}

void CGameObject::Set_Dead()
{
    m_isDead = true;
    Set_UpdatePhaseAllActive(false);
}

bool CGameObject::Get_IsUpdatePhaseActive(UPDATE_PHASE ePhase) const
{
    const size_t iPhase = static_cast<size_t>(ePhase);
    return iPhase < m_isUpdatePhaseActive.size() && m_isUpdatePhaseActive[iPhase];
}

void CGameObject::Set_UpdatePhaseActive(UPDATE_PHASE ePhase, bool isActive)
{
    const size_t iPhase = static_cast<size_t>(ePhase);
    if (iPhase < m_isUpdatePhaseActive.size())
        m_isUpdatePhaseActive[iPhase] = isActive;
}

void CGameObject::Set_UpdatePhaseAllActive(bool isActive)
{
    m_isUpdatePhaseActive.fill(isActive);
}

void CGameObject::Free()
{
    for (auto& ComponentPair : m_mapComponent)
        ComponentPair.second->Release();
    m_mapComponent.clear();

    if (m_pGameManager != nullptr)
        m_pGameManager->Release();
    m_pGameManager = nullptr;

    m_pDevice = nullptr;
    m_pDeviceContext = nullptr;
    CBase::Free();
}

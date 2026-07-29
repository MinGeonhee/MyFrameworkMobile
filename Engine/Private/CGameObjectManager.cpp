#include "pch.h"
#include "CGameObjectManager.h"
#include "CGameManager.h"
#include "CGameObject.h"
#include "CLayer.h"

CGameObjectManager::CGameObjectManager(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}, m_pDeviceContext{pDeviceContext}
{
}

HRESULT CGameObjectManager::Initialize(uint32 iTotalLevelCount)
{
    if (iTotalLevelCount == 0) return E_INVALIDARG;
    m_iTotalLevelCount = iTotalLevelCount;
    m_LevelLayers.resize(iTotalLevelCount);
    return S_OK;
}

CLayer* CGameObjectManager::Find_Layer(uint32 iLevelID, const std::wstring& strLayerTag) const
{
    if (iLevelID >= m_LevelLayers.size()) return nullptr;
    const auto Iter = m_LevelLayers[iLevelID].find(strLayerTag);
    return Iter != m_LevelLayers[iLevelID].end() ? Iter->second : nullptr;
}

CGameObject* CGameObjectManager::Find_GameObject(uint32 iLevelID,
                                                 const std::wstring& strLayerTag,
                                                 const std::wstring& strGameObjectName) const
{
    CLayer* pLayer = Find_Layer(iLevelID, strLayerTag);
    return pLayer != nullptr ? pLayer->Find_GameObject(strGameObjectName) : nullptr;
}

CGameObject* CGameObjectManager::Find_GameObject(uint32 iLevelID,
                                                 const std::wstring& strLayerTag,
                                                 CGameObject* pGameObject) const
{
    CLayer* pLayer = Find_Layer(iLevelID, strLayerTag);
    return pLayer != nullptr ? pLayer->Find_GameObject(pGameObject) : nullptr;
}

CGameObject* CGameObjectManager::Add_GameObject(uint32 iPrototypeLevelID,
                                                const std::wstring& strPrototypeTag,
                                                uint32 iCloneLevelID,
                                                const std::wstring& strLayerTag,
                                                void* pArg)
{
    CGameObject* pObject = static_cast<CGameObject*>(CGameManager::GetInstance()->PrototypeManager_Clone_Prototype(
        PROTOTYPE_TYPE::GAMEOBJECT, iPrototypeLevelID, strPrototypeTag, pArg));
    if (pObject == nullptr) return nullptr;
    if (FAILED(Add_GameObject_ToLayer(iCloneLevelID, strLayerTag, pObject)))
    {
        pObject->Release();
        return nullptr;
    }
    return pObject;
}

HRESULT CGameObjectManager::Add_GameObject_ToLayer(uint32 iLevelID,
                                                   const std::wstring& strLayerTag,
                                                   CGameObject* pGameObject)
{
    if (iLevelID >= m_LevelLayers.size() || strLayerTag.empty() || pGameObject == nullptr)
        return E_INVALIDARG;
    CLayer* pLayer = Find_Layer(iLevelID, strLayerTag);
    if (pLayer == nullptr)
    {
        pLayer = CLayer::Create();
        if (pLayer == nullptr) return E_FAIL;
        m_LevelLayers[iLevelID].emplace(strLayerTag, pLayer);
    }
    return pLayer->Add_GameObject(pGameObject);
}

void CGameObjectManager::Pre_Update(uint32 iLevelID, float fTimeDelta)
{
    if (iLevelID >= m_LevelLayers.size()) return;
    for (auto& Pair : m_LevelLayers[iLevelID]) Pair.second->Pre_Update(fTimeDelta);
}
void CGameObjectManager::Update(uint32 iLevelID, float fTimeDelta)
{
    if (iLevelID >= m_LevelLayers.size()) return;
    for (auto& Pair : m_LevelLayers[iLevelID]) Pair.second->Update(fTimeDelta);
}
void CGameObjectManager::Late_Update(uint32 iLevelID, float fTimeDelta)
{
    if (iLevelID >= m_LevelLayers.size()) return;
    for (auto& Pair : m_LevelLayers[iLevelID]) Pair.second->Late_Update(fTimeDelta);
}
HRESULT CGameObjectManager::Add_RenderGroup(uint32 iLevelID)
{
    if (iLevelID >= m_LevelLayers.size()) return E_INVALIDARG;
    for (auto& Pair : m_LevelLayers[iLevelID]) if (FAILED(Pair.second->Add_RenderGroup())) return E_FAIL;
    return S_OK;
}
void CGameObjectManager::Add_ColliderGroup(uint32 iLevelID)
{
    if (iLevelID >= m_LevelLayers.size()) return;
    for (auto& Pair : m_LevelLayers[iLevelID]) Pair.second->Add_ColliderGroup();
}
void CGameObjectManager::Clean_DeadObject()
{
    for (auto& Level : m_LevelLayers) for (auto& Pair : Level) Pair.second->Clean_DeadObject();
}

HRESULT CGameObjectManager::Clear_GameObjectByLevelID(uint32 iLevelID)
{
    // Index 0은 Global이므로 일반 Scene 전환에서 지우지 않는다.
    if (iLevelID == Engine::GLOBAL_LEVEL || iLevelID >= m_LevelLayers.size()) return E_INVALIDARG;
    for (auto& Pair : m_LevelLayers[iLevelID]) Pair.second->Release();
    m_LevelLayers[iLevelID].clear();
    return S_OK;
}

const std::map<std::wstring, CLayer*>* CGameObjectManager::Get_LevelLayers(uint32 iLevelID) const
{
    return iLevelID < m_LevelLayers.size() ? &m_LevelLayers[iLevelID] : nullptr;
}
CLayer* CGameObjectManager::Get_Layer(uint32 iLevelID, const std::wstring& strLayerTag) const
{
    return Find_Layer(iLevelID, strLayerTag);
}

CGameObjectManager* CGameObjectManager::Create(uint32 iTotalLevelCount, Device* pDevice, Context* pDeviceContext)
{
    CGameObjectManager* pInstance = new CGameObjectManager{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(iTotalLevelCount))) { pInstance->Release(); return nullptr; }
    return pInstance;
}

void CGameObjectManager::Free()
{
    for (auto& Level : m_LevelLayers)
    {
        for (auto& Pair : Level) Pair.second->Release();
        Level.clear();
    }
    m_LevelLayers.clear();
    m_pDevice = nullptr;
    m_pDeviceContext = nullptr;
    CBase::Free();
}

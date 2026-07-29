#include "pch.h"
#include "CPrototypeManager.h"

#include "CComponent.h"
#include "CGameObject.h"
#include "CVIBuffer_Rect.h"
#include "CVIBuffer_Terrain.h"
#include "CVIBuffer_Cube.h"
#include "CSpriteAnimator.h"
#include "CTransform.h"
#include "CDefaultCameraObject.h"

CPrototypeManager::CPrototypeManager(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
{
}

CPrototypeManager::~CPrototypeManager()
{
    Free();
}

HRESULT CPrototypeManager::Initialize(const Engine::PrototypeManager_Desc& Desc)
{
    if (Desc.iTotalLevelCount == 0)
        return E_INVALIDARG;

    m_iTotalLevelCount = Desc.iTotalLevelCount;
    for (auto& PrototypeMapByType : m_PrototypeMaps)
        PrototypeMapByType.resize(m_iTotalLevelCount);

    return S_OK;
}

HRESULT CPrototypeManager::Ready_DefaultPrototype()
{
    CTransform* pTransformPrototype=CTransform::Create(m_pDevice,m_pDeviceContext);
    if(!pTransformPrototype||FAILED(Add_Prototype(PROTOTYPE_TYPE::COMPONENT,Engine::GLOBAL_LEVEL,L"Proto_Component_Transform",pTransformPrototype))){if(pTransformPrototype)pTransformPrototype->Release();return E_FAIL;}

    CVIBuffer_Rect* pRectPrototype = CVIBuffer_Rect::Create(m_pDevice, m_pDeviceContext);
    if (pRectPrototype == nullptr)
        return E_FAIL;

    if (FAILED(Add_Prototype(
            PROTOTYPE_TYPE::COMPONENT,
            Engine::GLOBAL_LEVEL,
            L"Proto_Component_VIBuffer_Rect",
            pRectPrototype)))
    {
        pRectPrototype->Release();
        return E_FAIL;
    }

    CVIBuffer_Terrain* pTerrainPrototype=CVIBuffer_Terrain::Create(m_pDevice,m_pDeviceContext);
    if(!pTerrainPrototype||FAILED(Add_Prototype(PROTOTYPE_TYPE::COMPONENT,Engine::GLOBAL_LEVEL,L"Proto_Component_VIBuffer_Terrain",pTerrainPrototype))){if(pTerrainPrototype)pTerrainPrototype->Release();return E_FAIL;}
    CVIBuffer_Cube* pCubePrototype=CVIBuffer_Cube::Create(m_pDevice,m_pDeviceContext);
    if(!pCubePrototype||FAILED(Add_Prototype(PROTOTYPE_TYPE::COMPONENT,Engine::GLOBAL_LEVEL,L"Proto_Component_VIBuffer_Cube",pCubePrototype))){if(pCubePrototype)pCubePrototype->Release();return E_FAIL;}

    CSpriteAnimator* pSpriteAnimatorPrototype = CSpriteAnimator::Create(m_pDevice, m_pDeviceContext);
    if (pSpriteAnimatorPrototype == nullptr || FAILED(Add_Prototype(
            PROTOTYPE_TYPE::COMPONENT,
            Engine::GLOBAL_LEVEL,
            L"Proto_Component_SpriteAnimator",
            pSpriteAnimatorPrototype)))
    {
        if (pSpriteAnimatorPrototype != nullptr) pSpriteAnimatorPrototype->Release();
        return E_FAIL;
    }

    CDefaultCameraObject* pCameraPrototype=CDefaultCameraObject::Create(m_pDevice,m_pDeviceContext);
    if(!pCameraPrototype||FAILED(Add_Prototype(PROTOTYPE_TYPE::GAMEOBJECT,Engine::GLOBAL_LEVEL,L"Proto_GameObject_DefaultCamera",pCameraPrototype))){if(pCameraPrototype)pCameraPrototype->Release();return E_FAIL;}

    return S_OK;
}

HRESULT CPrototypeManager::Add_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                         uint32 iLevelID,
                                         const std::wstring& strPrototypeTag,
                                         CBase* pPrototype)
{
    if (ePrototypeType >= PROTOTYPE_TYPE::END || iLevelID >= m_iTotalLevelCount ||
        strPrototypeTag.empty() || pPrototype == nullptr)
        return E_INVALIDARG;

    auto& PrototypeMap = m_PrototypeMaps[static_cast<size_t>(ePrototypeType)][iLevelID];
    if (PrototypeMap.find(strPrototypeTag) != PrototypeMap.end())
        return E_FAIL;

    PrototypeMap.emplace(strPrototypeTag, pPrototype);
    return S_OK;
}

CBase* CPrototypeManager::Clone_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                          uint32 iLevelID,
                                          const std::wstring& strPrototypeTag,
                                          void* pArg) const
{
    CBase* pPrototype = Find_Prototype(ePrototypeType, iLevelID, strPrototypeTag);
    if (pPrototype == nullptr)
        return nullptr;

    switch (ePrototypeType)
    {
    case PROTOTYPE_TYPE::GAMEOBJECT:
        return static_cast<CGameObject*>(pPrototype)->Clone(pArg);

    case PROTOTYPE_TYPE::COMPONENT:
        return static_cast<CComponent*>(pPrototype)->Clone(pArg);

    default:
        return nullptr;
    }
}

void CPrototypeManager::Clear_Prototype(uint32 iLevelID)
{
    if (iLevelID == Engine::GLOBAL_LEVEL || iLevelID >= m_iTotalLevelCount)
        return;

    for (auto& PrototypeMapByType : m_PrototypeMaps)
    {
        auto& PrototypeMap = PrototypeMapByType[iLevelID];
        for (auto& PrototypePair : PrototypeMap)
            PrototypePair.second->Release();
        PrototypeMap.clear();
    }
}

const std::map<std::wstring, CBase*>* CPrototypeManager::Get_PrototypeList(
    PROTOTYPE_TYPE ePrototypeType,
    uint32 iLevelID) const
{
    if (ePrototypeType >= PROTOTYPE_TYPE::END || iLevelID >= m_iTotalLevelCount)
        return nullptr;

    return &m_PrototypeMaps[static_cast<size_t>(ePrototypeType)][iLevelID];
}

CBase* CPrototypeManager::Find_Prototype(PROTOTYPE_TYPE ePrototypeType,
                                         uint32 iLevelID,
                                         const std::wstring& strPrototypeTag) const
{
    const auto* pPrototypeMap = Get_PrototypeList(ePrototypeType, iLevelID);
    if (pPrototypeMap == nullptr)
        return nullptr;

    const auto Iter = pPrototypeMap->find(strPrototypeTag);
    return Iter != pPrototypeMap->end() ? Iter->second : nullptr;
}

CPrototypeManager* CPrototypeManager::Create(const Engine::PrototypeManager_Desc& Desc,
                                              Device* pDevice,
                                              Context* pDeviceContext)
{
    CPrototypeManager* pManager = new CPrototypeManager{pDevice, pDeviceContext};
    if (FAILED(pManager->Initialize(Desc)))
    {
        delete pManager;
        return nullptr;
    }

    return pManager;
}

void CPrototypeManager::Free()
{
    for (auto& PrototypeMapByType : m_PrototypeMaps)
    {
        for (auto& PrototypeMap : PrototypeMapByType)
        {
            for (auto& PrototypePair : PrototypeMap)
                PrototypePair.second->Release();
            PrototypeMap.clear();
        }
        PrototypeMapByType.clear();
    }

    m_iTotalLevelCount = 0;
    m_pDevice = nullptr;
    m_pDeviceContext = nullptr;
    CBase::Free();
}

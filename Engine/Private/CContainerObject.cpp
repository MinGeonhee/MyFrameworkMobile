#include "CContainerObject.h"
#include "CGameManager.h"
#include "CPartObject.h"


CContainerObject::CContainerObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CGameObject(pDevice, pContext), m_mapPartObject{}
{
    m_isPartObject = false;
}

CContainerObject::CContainerObject(const CContainerObject& Proto)
    : CGameObject(Proto), m_mapPartObject(Proto.m_mapPartObject)
{
    m_isPartObject = false;
}

HRESULT CContainerObject::Proto_Initialize()
{
    if (FAILED(__super::Proto_Initialize()))
        return E_FAIL;

    return S_OK;
}

HRESULT CContainerObject::Clone_Initialize(void* pArg)
{
    if (FAILED(__super::Clone_Initialize(pArg)))
        return E_FAIL;


    if (FAILED(CContainerObject::Ready_Component(pArg)))
        return E_FAIL;

    return S_OK;
}

HRESULT CContainerObject::Ready_Component(void* pArg)
{
    return S_OK;
}

HRESULT CContainerObject::Ready_PartObject()
{
    return S_OK;
}

void CContainerObject::Pre_Update(float DT)
{
    __super::Pre_Update(DT);

    for (auto& PartObject : m_mapPartObject)
        if (PartObject.second)
            if (PartObject.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::PreUpdate))
                PartObject.second->Pre_Update(DT);
}

void CContainerObject::Update(float DT)
{
    __super::Update(DT);


    for (auto& PartObject : m_mapPartObject)
        if (PartObject.second)
            if (PartObject.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::Update))
                PartObject.second->Update(DT);
}

void CContainerObject::Late_Update(float DT)
{
    __super::Late_Update(DT);


    for (auto& PartObject : m_mapPartObject)
        if (PartObject.second)
            if (PartObject.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::LateUpdate))
                PartObject.second->Late_Update(DT);
}

HRESULT CContainerObject::Add_RenderGroup()
{
    //__super::Add_RenderGroup();

    //for (auto& PartObject : m_mapPartObject)
    //    if (PartObject.second)
    //        if (PartObject.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::Add_RenderGroup))
    //                PartObject.second->Add_RenderGroup();

    return S_OK;
}

HRESULT CContainerObject::Render(FD CCameraObjectBase* pRenderCam)
{
    for (auto& PartObject : m_mapPartObject)
        if (PartObject.second)
            if (PartObject.second->Get_IsUpdatePhaseActive(UPDATE_PHASE::Render))
                if (FAILED(PartObject.second->Render(pRenderCam)))
                    return E_FAIL;

    return S_OK;
}

void CContainerObject::Set_Dead()
{
    __super::Set_Dead();

    for (auto& PartObject : m_mapPartObject)
        if (PartObject.second)
            PartObject.second->Set_Dead();
}



HRESULT CContainerObject::Add_PartObject(uint32 iFindSceneID, const wstring& wstrFindProtoKey, const wstring& wstrAddKey, void* pArg, CPartObject** ppPartObject)
{
    CPartObject* pPartsObject{ nullptr };
    if (iFindSceneID == GLOBAL) pPartsObject = static_cast<CPartObject*>(m_pGameManager->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::GAMEOBJECT, wstrFindProtoKey, pArg));
    else pPartsObject = static_cast<CPartObject*>(m_pGameManager->PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE::GAMEOBJECT, iFindSceneID, wstrFindProtoKey, pArg));
    
    NULLPTR_RETURN_EFAIL(pPartsObject);

    m_mapPartObject.emplace(wstrAddKey,pPartsObject);
   
   
    *ppPartObject = pPartsObject;
    Safe_AddRef(*ppPartObject);

    return S_OK;
}

CPartObject* CContainerObject::Find_PartObject(const wstring& wstrFindKey)
{
    const auto& iter = m_mapPartObject.find(wstrFindKey);
    if (iter == m_mapPartObject.end())
        return nullptr;

    return iter->second;
}

void CContainerObject::Free()
{
    __super::Free();
    
    /* CContainerObejct::Free() */
    for (auto& PartObject : m_mapPartObject)
        Safe_Release(PartObject.second);
}


#ifdef _DEBUG

void CContainerObject::ImGuiUpdate()
{
    /* CGameObject Update */
    __super::ImGuiUpdate();

    if (ImGui::CollapsingHeader(" Container Object Info "))
    {
        /* CContainerObject Update */
        for (auto& PartObject : m_mapPartObject)
        {
            if (ImGui::TreeNode(WString_To_String(PartObject.first).c_str()))
            {
                PartObject.second->ImGuiUpdate();
                ImGui::TreePop();
            }
        }
    }
}

#endif



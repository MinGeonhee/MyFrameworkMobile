#include "CLayer.h"
#include "CGameObject.h"

CLayer::CLayer()
{
}


HRESULT CLayer::Initialize()
{
    /* 메모리 재할당 방지를 위해 100개정도 넉넉하게 잡아주기 */
    m_vecGameObject.reserve(100);

    return S_OK;
}

HRESULT CLayer::Add_GameObject(CGameObject* pGameObject)
{
    m_vecGameObject.push_back(pGameObject);

    return S_OK;
}

CGameObject* CLayer::Find_GameObject(CGameObject* pGameObject)
{
    for (const auto& GameObject : m_vecGameObject)
        if (GameObject == pGameObject)
            return GameObject;
    
    return nullptr;
}

CGameObject* CLayer::Find_GameObject(const wstring& strGameObjectName)
{
    if (m_vecGameObject.empty())
        return nullptr;

    for (const auto& GameObject : m_vecGameObject)
        if (GameObject->GetName() == strGameObjectName)
            return GameObject;

    return nullptr;
}

void    CLayer::Pre_Update(float DT)
{
    /* Pre Update */
    for (auto& GameObject : m_vecGameObject) 
        if (GameObject)
            if (GameObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::PreUpdate) == true)
                GameObject->Pre_Update(DT);
    return;
}
void    CLayer::Update(float DT)
{
    /* Update */
    for (auto& GameObject : m_vecGameObject)
        if (GameObject)
            if (GameObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Update) == true)
                GameObject->Update(DT);

    return;
}
void    CLayer::Late_Update(float DT)
{
    for (auto& GameObject : m_vecGameObject)
        if( GameObject)
            if (GameObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::LateUpdate) == true)
                GameObject->Late_Update(DT);
    return;
}

HRESULT CLayer::Add_RenderGroup()
{
    /*  Vector 형태로 관리하기 떄문에 중간삭제를 하지 않는다 따라서 중간에 nullptr이 껴있을 수 있다. */
    for (auto& GameObject : m_vecGameObject)
        if (GameObject)
            if (GameObject->Get_IsUpdatePhaseActive(UPDATE_PHASE::Add_RenderGroup) == true)
                if (FAILED(GameObject->Add_RenderGroup()))
                    return E_FAIL;

    return S_OK;
}



void    CLayer::Render()
{
    /* Render */
    return;
}

HRESULT CLayer::ClearGameObject()
{
    for (auto& pGameObject : m_vecGameObject)
        Safe_Release(pGameObject);

    /* nullptr 로 들어가있던 애들 다 날리기 Size는 줄어들지만 Capacity 는 그대로 있다. */
    m_vecGameObject.clear();

    return S_OK;
}



void    CLayer::Add_ColliderGroup()
{
    for (auto& pGameObject : m_vecGameObject)
        if (pGameObject)
            if (!pGameObject->Get_Dead())
                pGameObject->Add_ColliderGroup();
}

void CLayer::Clean_DeadObject()
{
    auto iter = std::remove_if(m_vecGameObject.begin(), m_vecGameObject.end(), [](CGameObject* pGameObject) ->bool {
        if (pGameObject->Get_Dead())
        {
            Safe_Release(pGameObject);
            return true;
        }
        return false;
        });
    m_vecGameObject.erase(iter,m_vecGameObject.end());
}


wstring CLayer::GetGameObjectName(uint32 iIndex)
{
    if (m_vecGameObject.empty())
        return L"EMPTY";
    if (m_vecGameObject.size() < iIndex)
        return L"EMPTY";

    return m_vecGameObject[iIndex]->GetName();
}

CLayer* CLayer::Create()
{
    CLayer* pLayer = new CLayer();

    INIT_FAILED_CEHCK(pLayer);

    return pLayer;
}

void    CLayer::Free()
{
    __super::Free();

    /* 내 Free() */
    for (auto& pGameObject : m_vecGameObject)
        Safe_Release(pGameObject);


    m_vecGameObject.clear();
    vector<CGameObject*>().swap(m_vecGameObject);
}

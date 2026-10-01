#include "CUIObject.h"

#include "CGameManager.h"
#include "CTransform.h"
#include "CVIBuffer_Rect.h"
#include "CCanvas.h"
#include "CUIComponent.h"

CUIObject::CUIObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice) , m_pDeviceContext(pContext), m_eDepth(UI_DEPTH::END) , m_pGameManager(CGameManager::GetInstance()), m_pUITransformCom(nullptr) , m_isActive(true)
    , m_funcEvent(nullptr), m_umapUIComponent{}, m_vecUIComponent{}
{
    Safe_AddRef(m_pGameManager);
}

HRESULT	CUIObject::Initialize(void* pArg)
{
    UIOBJECT_DESC* pDesc = static_cast<UIOBJECT_DESC*>(pArg);
    m_wstrName  =   pDesc->wstrName;
    m_eDepth    =   pDesc->eUIDepth;
    m_funcEvent =   pDesc->funcEvent;

    if (FAILED(CUIObject::Ready_Component(pArg)))
        return E_FAIL;

    return S_OK;
}


void CUIObject::Set_UITransformMoveEvent(bool isMove)
{
    m_pUITransformCom->Set_IsMoveEvent(isMove);
}

void    CUIObject::Set_IsActive(bool isActive)
{
    m_isActive = isActive;
    return;
}

bool CUIObject::Get_UITarnsformMoveEvent()
{
    return m_pUITransformCom->Get_IsMoveEvent();
}

HRESULT CUIObject::Ready_Component(void* pArg)
{
    if(FAILED(CUIObject::Add_UIComponent(GLOBAL, L"Proto_UIComponent_UITransform", L"UICom_UITransform", pArg, reinterpret_cast<CUIComponent**>(&m_pUITransformCom))))
        return E_FAIL;
    return S_OK;
}

HRESULT	CUIObject::Add_UIComponent(uint32 iFindSceneID, CONST std::wstring& wstrFindProtoName, CONST std::wstring& wstrAddName, void* pArg, FD CUIComponent** ppOut)
{
    CUIComponent* pCom{ nullptr };

    if (iFindSceneID == GLOBAL)
        pCom = static_cast<CUIComponent*>(m_pGameManager->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, wstrFindProtoName, pArg));
    else
        pCom = static_cast<CUIComponent*>(m_pGameManager->PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE::COMPONENT, iFindSceneID, wstrFindProtoName, pArg));

    NULLPTR_RETURN_EFAIL(pCom);

    m_umapUIComponent.emplace(wstrAddName, pCom);
    m_vecUIComponent.push_back(pCom);
    Safe_AddRef(pCom);

    pCom->Set_Owner(this);

    if (ppOut != nullptr)
    {
        *ppOut = pCom;
        Safe_AddRef(pCom);
    }

    return S_OK;
}


CUIComponent* CUIObject::Get_UIComponent(const wstring& wstrFindKey)
{
    auto iter = m_umapUIComponent.find(wstrFindKey);
    
    if (iter == m_umapUIComponent.end()) return nullptr;


    return iter->second;
}



void CUIObject::Pre_Update(CCanvas* pCanvas,float DT)
{
    for (auto& UICom : m_vecUIComponent)
        if (UICom)
            if(UICom->Get_IsActive())
                UICom->Pre_Update(DT);
}


void CUIObject::Update(CCanvas* pCanvas,float DT)
{
    for (auto& UICom : m_vecUIComponent)
        if (UICom)
            if (UICom->Get_IsActive())
                UICom->Update(DT);
}


void CUIObject::Late_Update(CCanvas* pCanvas,float DT)
{
    for (auto& UICom : m_vecUIComponent)
        if (UICom)
            if (UICom->Get_IsActive())
                UICom->Late_Update(DT);

    m_pUITransformCom->Update_World(pCanvas);
}


HRESULT CUIObject::Render(CCanvas* pCanvas)
{
    for (auto& UICom : m_vecUIComponent)
        if (UICom)
            if (UICom->Get_IsActive())
                if(FAILED(UICom->Render(pCanvas)))
                    return E_FAIL;

    return S_OK;
}


CUIObject* CUIObject::Create(void* pArg, ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CUIObject* pUIObject = new CUIObject(pDevice, pContext);

    if (FAILED(pUIObject->Initialize(pArg)))
    {
        Safe_Release(pUIObject);
        MSG_BOX("UIObject is failed to Create");
        return nullptr;
    }

    return pUIObject;
}


void CUIObject::Free()
{
    __super::Free();
    /* UI Object Free() */

    for (auto& UICom : m_umapUIComponent)
        Safe_Release(UICom.second);

    for (auto& UICom : m_vecUIComponent)
        Safe_Release(UICom);

    Safe_Release(m_pUITransformCom);

    Safe_Release(m_pGameManager);
}

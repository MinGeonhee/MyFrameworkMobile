#include "CGameObject.h"
#include "CGameManager.h"
#include "CLayer.h"
#include "CComponent.h"
#include "CTransform.h"
#include "CImGuiFuntion.h"
#include "CFileFunction.h"
#include "CJsonFunction.h"
#include "CCollider.h"
#include "CGameObjectInfoWindow.h"
#include "CUIObject.h"

CGameObject::CGameObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice), m_pDeviceContext(pContext), m_strName{}, m_pGameManager(CGameManager::GetInstance()), m_mapComponent{} , m_pTransformCom(nullptr)
    , m_pTargetObject(nullptr), m_eMakeType(MAKE_TYPE::PROTOTYPE), m_vecColliderCom{}, m_isExpectedDelete{ false }, m_iIndex{ NONINDEX }, m_iEventIndex{ No_EventIndex }
    , m_isRenderCulling(true), m_isDead(false) , m_pCollisionObject(nullptr) , m_iMainType(-1) , m_isPartObject(false)
{
    Safe_AddRef(m_pGameManager);

    for (int i = 0; i < Cast_Uint(UPDATE_PHASE::END); ++i)
        m_isUpdatePahseActive[i] = true;
 
}

CGameObject::CGameObject(const CGameObject& pGameObject)
    : m_pDevice(pGameObject.m_pDevice), m_pDeviceContext(pGameObject.m_pDeviceContext), m_pGameManager(pGameObject.m_pGameManager)
    , m_pTransformCom(nullptr), m_pTargetObject(pGameObject.m_pTargetObject), m_eMakeType(MAKE_TYPE::CLONE), m_vecColliderCom{}
    , m_iIndex(pGameObject.m_iIndex), m_isExpectedDelete(pGameObject.m_isExpectedDelete), m_iEventIndex{ pGameObject.m_iEventIndex }
    , m_isRenderCulling(pGameObject.m_isRenderCulling), m_isDead(pGameObject.m_isDead), m_pCollisionObject(pGameObject.m_pCollisionObject)
    , m_iMainType(pGameObject.m_iMainType) , m_isPartObject(pGameObject.m_isPartObject)
{
    Safe_AddRef(m_pGameManager);

    for (int i = 0; i < Cast_Uint(UPDATE_PHASE::END); ++i)
    {
        m_isUpdatePahseActive[i] = pGameObject.m_isUpdatePahseActive[i];
    }
}

CGameObject::~CGameObject()
{
}



HRESULT CGameObject::Proto_Initialize()
{
    return S_OK;
}

HRESULT CGameObject::Clone_Initialize(void* pArg)
{
    NULLPTR_RETURN_EFAIL(pArg);

    GameObject_Desc* pGameObject_Desc
        = static_cast<GameObject_Desc*>(pArg);

    m_strName = pGameObject_Desc->GameObect_Desc_strGameObjectName;
    m_iEventIndex = pGameObject_Desc->iEventIndex;
    /* 복사생성할떄 컴포넌트 복사해서 주기 */
    /* 기본적으로 엔진 단위에 있는 컴포넌트 들은 Global Proto에 있는 컴포넌트를 사용하면 될 거 같은데...*/

    if (FAILED(CGameObject::Ready_Component(pArg)))
        return E_FAIL;


    return S_OK;
}

HRESULT CGameObject::Ready_Component(void* pArg)
{
    if (FAILED(CGameObject::Add_Component(GLOBAL, L"Proto_Component_Transform", L"Com_Transform", pArg, Cast_AddCom(m_pTransformCom))))
        return E_FAIL;

    return S_OK;
}


HRESULT CGameObject::Add_Component(uint32 iFindSceneID, CONST std::wstring& wstrFindProtoName, CONST std::wstring& wstrAddName ,void* pArg, FD CComponent** ppOut)
{
    CComponent* pCom{ nullptr };

    if (iFindSceneID == GLOBAL)
        pCom = static_cast<CComponent*>(m_pGameManager->PrototypeManager_Clone_Prototype_InGlobal(PROTOTYPE_TYPE::COMPONENT, wstrFindProtoName, pArg));
    else
        pCom = static_cast<CComponent*>(m_pGameManager->PrototypeManager_Clone_Prototype(PROTOTYPE_TYPE::COMPONENT, iFindSceneID , wstrFindProtoName,pArg));

    NULLPTR_RETURN_EFAIL(pCom);

    m_mapComponent.emplace(wstrAddName,pCom);
    pCom->Set_Owner(this);

    *ppOut = pCom;
    Safe_AddRef(pCom);
    return S_OK;
}


void CGameObject::Pre_Update(float DT)
{
    return;
}

void CGameObject::Update(float DT)
{
    return;
}

void CGameObject::Late_Update(float DT)
{
    return;
}

HRESULT CGameObject::Add_RenderGroup()
{
    return S_OK;
}

HRESULT CGameObject::Render(FD CCameraObjectBase* pRenderCam)
{
    return S_OK;
}


#ifdef _DEBUG


void CGameObject::ImGuiUpdate()
{
    NULLPTR_RETURN_NULL(this);

    /* Name */
    CImGuiFuntion::RenderGameObjectName(this);

    /* Name Fix */
    CImGuiFuntion::FixName(&this->m_strName);

    /* Event Path 값 넣어주기 */
    ImGui::InputInt( " Event Path => [ %d ] " ,&m_iEventIndex);


    /* Target Object */
    ImGui::Text(" -- Target Obejct Information --");
    if (m_pTargetObject == nullptr)
        ImGui::Text("Nullptr");
    else
    {
        CImGuiFuntion::RenderName(m_pTargetObject->m_strName);
    }

    /* Update Active */
    if (ImGui::TreeNode("-- Update Active List --"))
    {
        for (uint32 i = 0; i < Cast_Uint(UPDATE_PHASE::END);++i)
        {
            string strNameTag = EnumUtils::UPDATE_PHASE_Enum_To_String((UPDATE_PHASE)i) + "###" + std::to_string(i);

            ImGui::Checkbox(strNameTag.c_str(), &m_isUpdatePahseActive[i]);

        }
        ImGui::TreePop();
    }



    if (m_mapComponent.empty())
    {
        CImGuiFuntion::ColorText(COLOR_RED, " Component is Eemty");
    }

    if (ImGui::CollapsingHeader(" Component list Info "))
    {
        for (auto& Component : m_mapComponent)
        {
            if (ImGui::TreeNode(WString_To_String(Component.first).c_str()))
            {
                Component.second->ImGuiUpdate();
                ImGui::TreePop();
            }
        }
        
    }
}

HRESULT CGameObject::Debug_ColliderAddRender()
{
    for (auto& Collider : m_vecColliderCom)
        if (Collider)
            if (FAILED(m_pGameManager->RenderManager_Add_DebugComponent(Collider)))
                return E_FAIL;

    return S_OK;
}

#endif // _DEBUG


void CGameObject::Collider_Update(const Float4x4* matWorld)
{
    if (!m_pTransformCom) return;

    for (auto& Collider : m_vecColliderCom)
        if (Collider)
            Collider->Update_World(matWorld);
}


bool CGameObject::Get_IsInCamArea(const Float4& vRenderCamAreaXZ)
{
    return m_pTransformCom->Get_IsInCamArea(vRenderCamAreaXZ);
}

void CGameObject::Set_Dead()
{
    m_isDead = true;

    for (auto& CCollider : m_vecColliderCom)
        if (CCollider)
            CCollider->Set_IsActive(false);

    CGameObject::Set_UpdatePhaseAllActive(false);
    m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSet(static_cast<float>(ANDROMEDA), static_cast<float>(ANDROMEDA), static_cast<float>(ANDROMEDA), 1.f));
}

void CGameObject::Respawn(const Float4& vRespawnPos)
{
    /* 일단 무책임하게 전부 true로 둘게 너가필요하면 함수내려서써 */
    for (auto& CCollider : m_vecColliderCom)
        if (CCollider)
            CCollider->Set_IsActive(true);

    m_pTransformCom->Set_State(Engine::STATE::POSITION, vRespawnPos );
    this->Set_UpdatePhaseAllActive(true);
}




const float CGameObject::Get_Z() const
{
    NULLPTR_RETURN_FLOAT(m_pTransformCom);

    return m_pTransformCom->Get_State_ToStore(STATE::POSITION).z;
}

bool CGameObject::Get_IsColliderActive(uint32 iIndex) const
{
    if (m_vecColliderCom[iIndex])
        return m_vecColliderCom[iIndex]->Get_IsActive();
    else
        return false;
}

void CGameObject::Set_ColliderActive(bool isActive,uint32 iIndex)
{
    if (m_vecColliderCom[iIndex])
        m_vecColliderCom[iIndex]->Set_IsActive(isActive);
}



void CGameObject::Semi_Update()
{
    if (m_pTransformCom)
    {
        for (auto& Collider : m_vecColliderCom)
            if (Collider)
                Collider->Update_World(m_pTransformCom->Get_matWorldPtr());
    }
}

void CGameObject::Set_AllColliderActive(bool isActive)
{
    for (auto& Collider : m_vecColliderCom)
        if (Collider)
            Collider->Set_IsActive(isActive);
}



void CGameObject::Free()
{
    __super::Free();

    for (auto& pComponent : m_mapComponent)
        Safe_Release(pComponent.second);

    for (auto& Collider : m_vecColliderCom)
        Safe_Release(Collider);

    Safe_Release(m_pTransformCom);
    Safe_Release(m_pGameManager);
}

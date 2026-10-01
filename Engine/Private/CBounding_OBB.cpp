#include "CBounding_OBB.h"
#include "CBounding_AABB.h"
#include "CBounding_Sphere.h"
#include "CShader.h"
#include "CGameObject.h"
#include "CTransform.h"
#include "MyImGuiHeader.h"
#include "DebugDraw.h"

CBounding_OBB::CBounding_OBB(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CBounding(pDevice, pContext),m_pLocalBoundingOrientedBox(nullptr) , m_pWorldBoundingOrientedBox(nullptr)
#ifdef _DEBUG
    , m_vRotation{}
#endif 
{
}


HRESULT CBounding_OBB::Initialize(void* pArg)
{
    CBounding_OBB::BOUNDING_OBB_DESC* pDesc = static_cast<CBounding_OBB::BOUNDING_OBB_DESC*>(pArg);

#ifdef _DEBUG
    m_vRotation = pDesc->vLocalRotation;
#endif // _DEBUG

    Float4 vQuatFloat{};
    XMStoreFloat4(&vQuatFloat, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vLocalRotation.x), XMConvertToRadians(pDesc->vLocalRotation.y), XMConvertToRadians(pDesc->vLocalRotation.z)));


    m_pLocalBoundingOrientedBox = new BoundingOrientedBox(pDesc->vLocalCenter,pDesc->vLocalExtents, vQuatFloat);
    m_pWorldBoundingOrientedBox = new BoundingOrientedBox(*m_pLocalBoundingOrientedBox);



    return S_OK;
}


bool CBounding_OBB::Intersects(CBounding* pOtherBounding,COLLISION_INFO* pOutInfo ,COLLIDER_TYPE eType)
{
    if (pOtherBounding == nullptr) return false;

    switch (eType)
    {
    case COLLIDER_TYPE::SPHERE: return m_pWorldBoundingOrientedBox->Intersects(*static_cast<CONST CBounding_Sphere*>(pOtherBounding)->Get_WorldBoundingSphere());
     	break;
    case COLLIDER_TYPE::AABB:   return m_pWorldBoundingOrientedBox->Intersects(*static_cast<CONST CBounding_AABB*>(pOtherBounding)->Get_WorldBoundingBox());
        break;
    case COLLIDER_TYPE::OBB:    return m_pWorldBoundingOrientedBox->Intersects(*static_cast<CONST CBounding_OBB*>(pOtherBounding)->Get_WorldBoundingOrientedBox());
        break;
    default:
        break;
    }

    return false;
}


bool CBounding_OBB::Intersects_Ray(const RAY_INFO& tRayInfo, RAY_HIT* pRayOut)
{
    return m_pWorldBoundingOrientedBox->Intersects(XMLoadFloat3(&tRayInfo.vRayOrigin), XMLoadFloat3(&tRayInfo.vRayDir), pRayOut->fDistance);;
}

void CBounding_OBB::Update_World(const Float4x4* pWorldMatrix)
{
    m_pLocalBoundingOrientedBox->Transform(*m_pWorldBoundingOrientedBox, XMLoadFloat4x4(pWorldMatrix));
}

void CBounding_OBB::Update_World(const XMatrix_1& WorldMatrix)
{
    m_pLocalBoundingOrientedBox->Transform(*m_pWorldBoundingOrientedBox,WorldMatrix);
}

Vector4 CBounding_OBB::Get_WorldPosition()
{
    return  XMVectorSetW(XMLoadFloat3(&m_pWorldBoundingOrientedBox->Center), 1.f);
}


CBounding_OBB* CBounding_OBB::Create(void* pArg,ComPtr<Device>pDevice, ComPtr<DeviceContext>pContext)
{

    CBounding_OBB* pBounding = new CBounding_OBB(pDevice, pContext);

    if (FAILED(pBounding->Initialize(pArg)))
    {
        Safe_Release(pBounding);

        MSG_BOX(" Bounding OBB is Failed to Clone");
    }
    return pBounding;

}

void CBounding_OBB::Free()
{
	__super::Free();

    Safe_Delete(m_pLocalBoundingOrientedBox);
    Safe_Delete(m_pWorldBoundingOrientedBox);
}




#ifdef _DEBUG
bool CBounding_OBB::ImGuiUpdate()
{
    bool isAction{false};
    /* Center */
    ImGui::Text("--- Center --- ");
    if (ImGui::InputFloat("X##1", &m_pLocalBoundingOrientedBox->Center.x, 0.1f)){isAction = true;};
    if(ImGui::InputFloat("Y##1", &m_pLocalBoundingOrientedBox->Center.y, 0.1f)) {isAction = true;};
    if(ImGui::InputFloat("Z##1", &m_pLocalBoundingOrientedBox->Center.z, 0.1f)) {isAction = true;};


    /* Extents */
    ImGui::Text("--- Extents --- ");
    if (ImGui::InputFloat("X##2", &m_pLocalBoundingOrientedBox->Extents.x, 0.1f))   { isAction = true;}
    if(ImGui::InputFloat("Y##2", &m_pLocalBoundingOrientedBox->Extents.y, 0.1f))    { isAction = true;}
    if(ImGui::InputFloat("Z##2", &m_pLocalBoundingOrientedBox->Extents.z, 0.1f))    { isAction = true;}

    /* Rotation */
    bool isWork{ false };
    ImGui::Text("--- Rotation --- ");
    if (ImGui::InputFloat("X##3", &m_vRotation.x, 0.1f)) { isWork = true; isAction = true;}
    if (ImGui::InputFloat("Y##3", &m_vRotation.y, 0.1f)) { isWork = true; isAction = true;}
    if (ImGui::InputFloat("Z##3", &m_vRotation.z, 0.1f)) { isWork = true; isAction = true;}

    if (isWork)
        XMStoreFloat4(&m_pLocalBoundingOrientedBox->Orientation,XMQuaternionRotationRollPitchYaw(XMConvertToRadians(m_vRotation.x), XMConvertToRadians(m_vRotation.y), XMConvertToRadians(m_vRotation.z)));

    return isAction;
}
void CBounding_OBB::Save_Bounding(nlohmann::ordered_json& json)
{
    Float3 vRotation = QuaternionToEulerDeg_Simple(XMLoadFloat4(&m_pLocalBoundingOrientedBox->Orientation));


    json["Center"]   = { { "X" , m_pLocalBoundingOrientedBox->Center.x}  , {"Y", m_pLocalBoundingOrientedBox->Center.y}  , {"Z", m_pLocalBoundingOrientedBox->Center.z} };
    json["Extents"]  = { { "X" , m_pLocalBoundingOrientedBox->Extents.x} , {"Y", m_pLocalBoundingOrientedBox->Extents.y} , {"Z", m_pLocalBoundingOrientedBox->Extents.z} };
    json["Rotation"] = { { "X" , m_vRotation.x} , {"Y", m_vRotation.y} , {"Z", m_vRotation.z} };
}

HRESULT CBounding_OBB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, COLLIDER_STATE eColliderState)
{
    switch (eColliderState)
    {
    case Engine::COLLIDER_STATE::ACTIVE:		DX::Draw(pBatch, *m_pWorldBoundingOrientedBox, XMVectorSet(0.f, 0.f, 1.f, 1.f)); break;
    case Engine::COLLIDER_STATE::NON_ACTIVE:	DX::Draw(pBatch, *m_pWorldBoundingOrientedBox, XMVectorSet(0.f, 1.f, 0.f, 1.f));break;
    case Engine::COLLIDER_STATE::COLLISION:		DX::Draw(pBatch, *m_pWorldBoundingOrientedBox, XMVectorSet(1.f, 0.f, 0.f, 1.f));break;
    default: return E_FAIL;
    }
    return S_OK;
}

HRESULT CBounding_OBB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, const Float4 vColor)
{
    DX::Draw(pBatch, *m_pWorldBoundingOrientedBox,XMLoadFloat4(&vColor));

    return S_OK;
}
#endif

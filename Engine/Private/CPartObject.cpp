#include "CPartObject.h"
#include "CContainerObject.h"
#include "CCameraObjectBase.h"
#include "CTransform.h"

CPartObject::CPartObject(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CGameObject(pDevice, pContext) , m_matCombinedWorld{} , m_pMasterWorldMatrix(nullptr) , m_pMasterContainerObject(nullptr)
{
    m_isPartObject = true;
}

CPartObject::CPartObject(const CPartObject& Proto)
    : CGameObject(Proto), m_matCombinedWorld(Proto.m_matCombinedWorld)
    , m_pMasterWorldMatrix(Proto.m_pMasterWorldMatrix), m_pMasterContainerObject(Proto.m_pMasterContainerObject)
{
    m_isPartObject = true;
}

HRESULT CPartObject::Proto_Initialize()
{
    if (FAILED(__super::Proto_Initialize()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPartObject::Clone_Initialize(void* pArg)
{
    if (FAILED(__super::Clone_Initialize(pArg)))
        return E_FAIL;

    PARTOBJECT_DESC* pDesc = static_cast<PARTOBJECT_DESC*>(pArg);
    m_pMasterContainerObject = pDesc->PartObjectDesc_pMasterContainerObject;
    /* Master World Ptr */
    m_pMasterWorldMatrix = m_pMasterContainerObject->GetTransform()->Get_matWorldPtr();

    if (FAILED(CPartObject::Ready_Component(pArg)))
        return E_FAIL;

    return S_OK;
}


HRESULT CPartObject::Ready_Component(void* pArg)
{
    return S_OK;
}

void CPartObject::Pre_Update(float DT)
{
    __super::Pre_Update(DT);
}

void CPartObject::Update(float DT)
{
    __super::Update(DT);

}

void CPartObject::Late_Update(float DT)
{
    __super::Late_Update(DT);
}

HRESULT CPartObject::Add_RenderGroup()
{
    return S_OK;
}

HRESULT CPartObject::Render(FD CCameraObjectBase* pRenderCam)
{
    return S_OK;
}

bool CPartObject::Get_IsInCamArea(const Float4& vRenderCamAreaXZ)
{
    return m_pMasterContainerObject->GetTransform()->Get_IsInCamArea(vRenderCamAreaXZ);
}

void CPartObject::SetUp_CombinedWorldMatrix(const XMatrix_1& ParentWorldMatrix)
{
    XMStoreFloat4x4(&m_matCombinedWorld, m_pTransformCom->Get_matWorld_ToCal() * ParentWorldMatrix);
}



void CPartObject::Free()
{
    __super::Free();

    /* CPartObject::Free() */
}

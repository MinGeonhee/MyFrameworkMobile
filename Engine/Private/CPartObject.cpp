#include "pch.h"
#include "CPartObject.h"
#include "CContainerObject.h"

namespace
{
void Identity(float* pMatrix)
{
    std::fill(pMatrix, pMatrix + 16, 0.f);
    pMatrix[0] = pMatrix[5] = pMatrix[10] = pMatrix[15] = 1.f;
}
}

CPartObject::CPartObject(Device* pDevice, Context* pDeviceContext)
    : CGameObject{pDevice, pDeviceContext}
{
    Identity(m_matCombinedWorld);
    Set_IsPartObject(true);
}

CPartObject::CPartObject(const CPartObject& Prototype)
    : CGameObject{Prototype}
{
    Identity(m_matCombinedWorld);
    Set_IsPartObject(true);
}

HRESULT CPartObject::Proto_Initialize() { return CGameObject::Proto_Initialize(); }

HRESULT CPartObject::Clone_Initialize(void* pArg)
{
    if (pArg == nullptr)
        return E_INVALIDARG;
    PARTOBJECT_DESC* pDesc = static_cast<PARTOBJECT_DESC*>(pArg);
    m_pMasterContainerObject = pDesc->pMasterContainerObject;
    m_pMasterWorldMatrix = pDesc->pParentWorldMatrix;
    return CGameObject::Clone_Initialize(pArg);
}

HRESULT CPartObject::Ready_Component(void* pArg) { return CGameObject::Ready_Component(pArg); }
void CPartObject::Pre_Update(float fTimeDelta) { CGameObject::Pre_Update(fTimeDelta); }
void CPartObject::Update(float fTimeDelta) { CGameObject::Update(fTimeDelta); }
void CPartObject::Late_Update(float fTimeDelta) { CGameObject::Late_Update(fTimeDelta); }
HRESULT CPartObject::Add_RenderGroup() { return CGameObject::Add_RenderGroup(); }
HRESULT CPartObject::Render() { return CGameObject::Render(); }

void CPartObject::SetUp_CombinedWorldMatrix(const float* pLocalWorldMatrix)
{
    if (pLocalWorldMatrix == nullptr) return;
    if (m_pMasterWorldMatrix == nullptr)
    {
        std::copy(pLocalWorldMatrix, pLocalWorldMatrix + 16, m_matCombinedWorld);
        return;
    }
    float Result[16]{};
    for (uint32 r=0;r<4;++r) for(uint32 c=0;c<4;++c) for(uint32 k=0;k<4;++k)
        Result[r*4+c] += pLocalWorldMatrix[r*4+k] * m_pMasterWorldMatrix[k*4+c];
    std::copy(std::begin(Result), std::end(Result), m_matCombinedWorld);
}

void CPartObject::Free()
{
    // Master는 PartObject를 소유하므로 역참조는 AddRef하지 않아 순환 참조를 만들지 않는다.
    m_pMasterContainerObject = nullptr;
    m_pMasterWorldMatrix = nullptr;
    CGameObject::Free();
}

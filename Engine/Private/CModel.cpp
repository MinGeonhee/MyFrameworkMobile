#include "pch.h"
#include "CModel.h"
#include "CMesh.h"
#include "CShader.h"
#include "CBone.h"
#include "CAnimation.h"

CModel::CModel(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext)
    : CResource{pDevice, pDeviceContext}
{
    m_eResourceType = RESOURCE_TYPE::MODEL;
}

CModel::CModel(const CModel& Prototype)
    : CResource{Prototype}
    , m_pOwner{nullptr}
    , m_strModelName{Prototype.m_strModelName}
    , m_eModelType{Prototype.m_eModelType}
{
    for (CMesh* pPrototypeMesh : Prototype.m_vecMesh)
    {
        CMesh* pMesh = pPrototypeMesh != nullptr ? pPrototypeMesh->Clone() : nullptr;
        if (pMesh != nullptr)
        {
            m_vecMesh.push_back(pMesh);
            m_mapMesh.emplace(pMesh->Get_Name(), pMesh);
        }
    }
    for (CBone* pBone : Prototype.m_vecBone)
        m_vecBone.push_back(pBone != nullptr ? pBone->Clone() : nullptr);
    for (CAnimation* pAnimation : Prototype.m_vecAnimation)
        m_vecAnimation.push_back(pAnimation != nullptr ? pAnimation->Clone() : nullptr);
    m_iCurrentAnimationIndex = Prototype.m_iCurrentAnimationIndex;
}

HRESULT CModel::Initialize(const std::wstring& strModelName, MODEL_TYPE eModelType)
{
    if (strModelName.empty() || eModelType >= MODEL_TYPE::END)
        return E_INVALIDARG;
    m_strModelName = strModelName;
    m_eModelType = eModelType;
    return S_OK;
}

HRESULT CModel::Copy_Initialize(void* pArg)
{
    // 원본처럼 Copy_Resource의 인자로 소유 GameObject를 전달할 수 있다.
    m_pOwner = static_cast<CGameObject*>(pArg);
    return S_OK;
}

HRESULT CModel::Add_Mesh(CMesh* pMesh)
{
    if (pMesh == nullptr || m_mapMesh.find(pMesh->Get_Name()) != m_mapMesh.end())
        return E_FAIL;
    pMesh->AddRef();
    m_vecMesh.push_back(pMesh);
    m_mapMesh.emplace(pMesh->Get_Name(), pMesh);
    return S_OK;
}

HRESULT CModel::Add_Bone(CBone* pBone)
{
    if (pBone == nullptr)
        return E_INVALIDARG;
    pBone->AddRef();
    m_vecBone.push_back(pBone);
    return S_OK;
}

HRESULT CModel::Add_Animation(CAnimation* pAnimation)
{
    if (pAnimation == nullptr)
        return E_INVALIDARG;
    pAnimation->AddRef();
    m_vecAnimation.push_back(pAnimation);
    return S_OK;
}

CMesh* CModel::Find_Mesh(const std::string& strName) const
{
    const auto Iter = m_mapMesh.find(strName);
    return Iter != m_mapMesh.end() ? Iter->second : nullptr;
}

CMesh* CModel::Get_Mesh(uint32 iIndex) const
{
    return iIndex < m_vecMesh.size() ? m_vecMesh[iIndex] : nullptr;
}

void CModel::Set_MeshActive(const std::string& strMeshName, bool isActive)
{
    CMesh* pMesh = Find_Mesh(strMeshName);
    if (pMesh != nullptr)
        pMesh->Set_Active(isActive);
}

void CModel::Set_MeshActive(uint32 iMeshIndex, bool isActive)
{
    CMesh* pMesh = Get_Mesh(iMeshIndex);
    if (pMesh != nullptr)
        pMesh->Set_Active(isActive);
}

HRESULT CModel::Draw(CShader* pShader, const std::string& strPassKey)
{
    if (pShader == nullptr)
        return E_INVALIDARG;
    for (CMesh* pMesh : m_vecMesh)
        if (pMesh != nullptr && FAILED(pMesh->Draw(pShader, strPassKey)))
            return E_FAIL;
    return S_OK;
}

bool CModel::Play_Animation(float fSpeed, float fTimeDelta)
{
    if (m_eModelType != MODEL_TYPE::ANIM || m_iCurrentAnimationIndex >= m_vecAnimation.size())
        return false;
    bool isEnd = false;
    m_vecAnimation[m_iCurrentAnimationIndex]->Update_TransformMatrix(fSpeed, fTimeDelta, m_vecBone, isEnd);
    for (CBone* pBone : m_vecBone)
        if (pBone != nullptr)
            pBone->Update_CombinedTransformationMatrix(m_vecBone);
    return isEnd;
}

void CModel::Change_Animation(uint32 iAnimationIndex, ANIMATION_PLAYTYPE ePlayType)
{
    if (iAnimationIndex >= m_vecAnimation.size())
        return;
    m_iCurrentAnimationIndex = iAnimationIndex;
    m_vecAnimation[iAnimationIndex]->SetPlayType(ePlayType);
    m_vecAnimation[iAnimationIndex]->ResetAnimation();
}

CModel* CModel::Create(const std::wstring& strModelName,
                       MODEL_TYPE eModelType,
                       Diligent::IRenderDevice* pDevice,
                       Diligent::IDeviceContext* pDeviceContext)
{
    CModel* pInstance = new CModel{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(strModelName, eModelType)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CResource* CModel::Copy_Resource(void* pArg)
{
    CModel* pCopy = new CModel{*this};
    if (FAILED(pCopy->Copy_Initialize(pArg)))
    {
        pCopy->Release();
        return nullptr;
    }
    return pCopy;
}

void CModel::Free()
{
    for (CAnimation* pAnimation : m_vecAnimation)
        if (pAnimation != nullptr)
            pAnimation->Release();
    for (CBone* pBone : m_vecBone)
        if (pBone != nullptr)
            pBone->Release();
    m_vecAnimation.clear();
    m_vecBone.clear();
    for (CMesh* pMesh : m_vecMesh)
        if (pMesh != nullptr)
            pMesh->Release();
    m_mapMesh.clear();
    m_vecMesh.clear();
    m_pOwner = nullptr;
    CResource::Free();
}

#include "pch.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CVIBuffer_Mesh.h"

CMesh::CMesh(const CMesh& Prototype)
    : CBase{Prototype}
    , m_isActive{Prototype.m_isActive}
    , m_strName{Prototype.m_strName}
    , m_pBuffer{Prototype.m_pBuffer != nullptr ? static_cast<CVIBuffer_Mesh*>(Prototype.m_pBuffer->Clone(nullptr)) : nullptr}
    , m_pMaterial{Prototype.m_pMaterial}
{
    if (m_pMaterial != nullptr)
        m_pMaterial->AddRef();
}

HRESULT CMesh::Initialize(const std::string& strName, CVIBuffer_Mesh* pBuffer, CMaterial* pMaterial)
{
    if (strName.empty() || pBuffer == nullptr)
        return E_INVALIDARG;
    m_strName = strName;
    m_pBuffer = pBuffer;
    m_pBuffer->AddRef();
    m_pMaterial = pMaterial;
    if (m_pMaterial != nullptr)
        m_pMaterial->AddRef();
    return S_OK;
}

HRESULT CMesh::Bind_Resource()
{
    return m_pBuffer != nullptr ? m_pBuffer->Bind_Resource() : E_FAIL;
}

HRESULT CMesh::Draw(CShader* pShader, const std::string& strPassKey)
{
    if (!m_isActive || pShader == nullptr || m_pBuffer == nullptr)
        return S_FALSE;
    if (m_pMaterial != nullptr)
        m_pMaterial->Set_GPU_Material(pShader);
    if (FAILED(pShader->Apply_Pass(strPassKey)))
        return E_FAIL;
    return m_pBuffer->Render();
}

CMesh* CMesh::Create(const std::string& strName, CVIBuffer_Mesh* pBuffer, CMaterial* pMaterial)
{
    CMesh* pInstance = new CMesh{};
    if (FAILED(pInstance->Initialize(strName, pBuffer, pMaterial)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CMesh* CMesh::Clone()
{
    return new CMesh{*this};
}

void CMesh::Free()
{
    if (m_pMaterial != nullptr)
        m_pMaterial->Release();
    if (m_pBuffer != nullptr)
        m_pBuffer->Release();
    m_pMaterial = nullptr;
    m_pBuffer = nullptr;
    CBase::Free();
}

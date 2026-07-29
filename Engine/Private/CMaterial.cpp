#include "pch.h"
#include "CMaterial.h"
#include "CShader.h"
#include "CTexture.h"

namespace
{
constexpr const char* MATERIAL_VARIABLES[] = {
    "g_DiffuseTexture", "g_AmbientTexture", "g_NormalTexture",
    "g_SpecularTexture", "g_EmissiveTexture", "g_MetalTexture"
};
}

CMaterial::CMaterial(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext)
    : CResource{pDevice, pDeviceContext}
{
    m_eResourceType = RESOURCE_TYPE::MATERIAL;
}

CMaterial::CMaterial(const CMaterial& Prototype)
    : CResource{Prototype}
    , m_strName{Prototype.m_strName}
    , m_arrayTexture{Prototype.m_arrayTexture}
{
    for (CTexture* pTexture : m_arrayTexture)
        if (pTexture != nullptr)
            pTexture->AddRef();
}

HRESULT CMaterial::Initialize(const std::wstring& strName)
{
    if (strName.empty())
        return E_INVALIDARG;
    m_strName = strName;
    return S_OK;
}

HRESULT CMaterial::Copy_Initialize(void* pArg)
{
    return CResource::Copy_Initialize(pArg);
}

HRESULT CMaterial::Set_Texture(MATERIAL_TYPE eType, CTexture* pTexture)
{
    const size_t iType = static_cast<size_t>(eType);
    if (iType >= m_arrayTexture.size())
        return E_INVALIDARG;
    if (pTexture != nullptr)
        pTexture->AddRef();
    if (m_arrayTexture[iType] != nullptr)
        m_arrayTexture[iType]->Release();
    m_arrayTexture[iType] = pTexture;
    return S_OK;
}

CTexture* CMaterial::Get_Texture(MATERIAL_TYPE eType) const
{
    const size_t iType = static_cast<size_t>(eType);
    return iType < m_arrayTexture.size() ? m_arrayTexture[iType] : nullptr;
}

HRESULT CMaterial::Set_GPU_Material(CShader* pShader)
{
    if (pShader == nullptr)
        return E_INVALIDARG;
    bool isAnyTexture = false;
    for (size_t i = 0; i < m_arrayTexture.size(); ++i)
    {
        if (m_arrayTexture[i] == nullptr)
            continue;
        // 원본의 Material SRV 배열 대신 의미가 드러나는 HLSL 변수 이름에 각각 연결한다.
        m_arrayTexture[i]->Set_GPU_Texture(pShader, MATERIAL_VARIABLES[i]);
        isAnyTexture = true;
    }
    return isAnyTexture ? S_OK : S_FALSE;
}

CMaterial* CMaterial::Create(const std::wstring& strName,
                             Diligent::IRenderDevice* pDevice,
                             Diligent::IDeviceContext* pDeviceContext)
{
    CMaterial* pInstance = new CMaterial{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(strName)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CResource* CMaterial::Copy_Resource(void* pArg)
{
    CMaterial* pCopy = new CMaterial{*this};
    if (FAILED(pCopy->Copy_Initialize(pArg)))
    {
        pCopy->Release();
        return nullptr;
    }
    return pCopy;
}

void CMaterial::Free()
{
    for (CTexture*& pTexture : m_arrayTexture)
    {
        if (pTexture != nullptr)
            pTexture->Release();
        pTexture = nullptr;
    }
    CResource::Free();
}

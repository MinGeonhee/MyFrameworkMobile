#include "CResourceManager.h"
#include "CGameManager.h"
#include "CResource.h"
#include "CShader.h"
#include "CTexture.h"
#include "CMaterial.h"
#include "CModel.h"

CResourceManager::CResourceManager(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : m_pDevice(pDevice) , m_pDeviceContext(pContext) , m_pGameManager(CGameManager::GetInstance())
{
    Safe_AddRef(m_pGameManager);
}

CResource* CResourceManager::Find_Resrouce(const wstring& wstrKey, RESOURCE_TYPE eResourceType)
{
   const  auto& iter = m_arrayResourceUMap[Cast_Uint(eResourceType)].find(wstrKey);
   if (iter == m_arrayResourceUMap[Cast_Uint(eResourceType)].end()) return nullptr;
   else return iter->second;
}

HRESULT CResourceManager::Ready_Default_Shader()
{
    CShader* pShader{ nullptr };

    pShader = CShader::Create(L"../../Engine/Shader/Shader_EngineUI.hlsl", VTXPOSTEX().GetInputElementDesc().data(), VTXPOSTEX().GetInputElementDescSize(), m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(pShader);
    m_arrayResourceUMap[Cast_Uint(RESOURCE_TYPE::SHADER)].emplace(Shader_EngineUI, pShader);

    pShader = CShader::Create(L"../../Engine/Shader/Shader_Deferred.hlsl", VTXPOSTEX().GetInputElementDesc().data(), VTXPOSTEX().GetInputElementDescSize(), m_pDevice, m_pDeviceContext);
    NULLPTR_RETURN_EFAIL(pShader);
    m_arrayResourceUMap[Cast_Uint(RESOURCE_TYPE::SHADER)].emplace(Shader_Deferred, pShader);
    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::TEXTURE,"g_DiffuseTexture");
    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::TEXTURE,"g_NormalTexture");
    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::TEXTURE,"g_ShadeTexture");
    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::TEXTURE,"g_DepthTexture");
    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::TEXTURE,"g_SpecularTexture");


    pShader->Add_EffectVariable(EFFECT_VAL_TYPE::MATRIX,"g_GlobalInvProjMatrix");


    return S_OK;
}

HRESULT CResourceManager::Ready_Default_Texture()
{
    if (FAILED(m_pGameManager->ResourceManager_Add_Resource(L"ScreenFade", CTexture::Create(L"../../Resource/UI/ScreenFade.jpg", 1, m_pDevice, m_pDeviceContext))))
        return E_FAIL;

    return S_OK;
}
HRESULT CResourceManager::Ready_Default_Material()
{
    return S_OK;
}
HRESULT CResourceManager::Ready_Default_Model()
{
    return S_OK;
}
HRESULT CResourceManager::Ready_DefaultResource()
{
    if (FAILED(Ready_Default_Shader()))
        return E_FAIL;

    if (FAILED(Ready_Default_Texture()))
        return E_FAIL;

    return S_OK;
}

CResource* CResourceManager::Get_Resource(const wstring& wstrKey, RESOURCE_TYPE eResourceType , bool isCopy,void* pArg)
{
    CResource* pFind = Find_Resrouce(wstrKey, eResourceType);
    NULLPTR_RETURN_NULLPTR(pFind);

    if (isCopy == false)
    {
        Safe_AddRef(pFind);
        return pFind;
    }
    else
        return pFind->Copy_Resource(pArg);
}

HRESULT  CResourceManager::Add_Resource(const wstring& wstrKey,CResource* pResource)
{
    NULLPTR_RETURN_EFAIL(pResource);

    RESOURCE_TYPE eType = pResource->Get_ResourceType();

    CResource* pFind = Find_Resrouce(wstrKey, eType);
    if (pFind != nullptr) return E_FAIL;
    
    m_arrayResourceUMap[Cast_Uint(eType)].emplace(wstrKey,pResource);

    return S_OK;
}

CResourceManager* CResourceManager::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    return new CResourceManager(pDevice, pContext);
}

void CResourceManager::Free()
{
    __super::Free();

    /* 원본삭제 */
    for (auto& ResourceMap : m_arrayResourceUMap)
        for (auto& Resource : ResourceMap)
            Safe_Release(Resource.second);

    Safe_Release(m_pGameManager);
}

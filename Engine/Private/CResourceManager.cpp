#include "pch.h"
#include "CResourceManager.h"

#include "CResource.h"
#include "CShader.h"
#include "CTexture.h"

namespace
{
constexpr char DEFAULT_RECT_VS[] = R"(
struct VSInput { float3 Position : ATTRIB0; float2 Texcoord : ATTRIB1; };
struct PSInput { float4 Position : SV_POSITION; float2 Texcoord : TEXCOORD0; };
cbuffer SpriteTransformConstants
{
    float4x4 g_WorldViewProjection;
    float4 g_TransformOption;
};
void main(in VSInput VSIn, out PSInput PSIn)
{
    float4 LocalPosition = float4(VSIn.Position, 1.0);
    PSIn.Position = g_TransformOption.x > 0.5
        ? mul(LocalPosition, g_WorldViewProjection)
        : LocalPosition;
    PSIn.Texcoord = VSIn.Texcoord;
}
)";

constexpr char DEFAULT_RECT_PS[] = R"(
struct PSInput { float4 Position : SV_POSITION; float2 Texcoord : TEXCOORD0; };
cbuffer SpriteConstants
{
    float4 g_UVRect;
};
Texture2D g_Texture;
SamplerState g_Texture_sampler;
float4 main(in PSInput PSIn) : SV_TARGET
{
    float2 UV = g_UVRect.xy + PSIn.Texcoord * g_UVRect.zw;
    return g_Texture.Sample(g_Texture_sampler, UV);
}
)";
constexpr char WORLD_COLOR_VS[] = R"(
struct VSInput { float3 Position : ATTRIB0; float2 Texcoord : ATTRIB1; };
cbuffer WorldConstants { float4x4 g_WorldViewProjection; float4 g_Color; };
void main(in VSInput I, out float4 Position : SV_POSITION) { Position=mul(float4(I.Position,1),g_WorldViewProjection); }
)";
constexpr char WORLD_COLOR_PS[] = R"(
cbuffer WorldConstants { float4x4 g_WorldViewProjection; float4 g_Color; };
float4 main() : SV_TARGET { return g_Color; }
)";
constexpr char SKY_VS[] = R"(
struct VSInput { float3 Position : ATTRIB0; float2 Texcoord : ATTRIB1; };
struct PSInput { float4 Position : SV_POSITION; float3 Direction : TEXCOORD0; };
cbuffer SkyConstants { float4x4 g_SkyViewProjection; };
void main(in VSInput I,out PSInput O){O.Position=mul(float4(I.Position,1),g_SkyViewProjection);O.Direction=I.Position;}
)";
constexpr char SKY_PS[] = R"(
struct PSInput { float4 Position : SV_POSITION; float3 Direction : TEXCOORD0; };
Texture2D g_Texture; SamplerState g_Texture_sampler;
float4 main(in PSInput I):SV_TARGET{float3 d=normalize(I.Direction);float2 uv=float2(atan2(d.z,d.x)*0.159154943+0.5,0.5-asin(clamp(d.y,-1,1))*0.318309886);return g_Texture.Sample(g_Texture_sampler,uv);}
)";
}

CResourceManager::CResourceManager(Diligent::IRenderDevice* pDevice,
                                   Diligent::IDeviceContext* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
{
}

CResource* CResourceManager::Find_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType) const
{
    const size_t iType = static_cast<size_t>(eResourceType);
    if (iType >= m_arrayResourceMap.size())
        return nullptr;
    const auto Iter = m_arrayResourceMap[iType].find(strKey);
    return Iter != m_arrayResourceMap[iType].end() ? Iter->second : nullptr;
}

HRESULT CResourceManager::Ready_Default_Shader()
{
    constexpr Engine::SHADER_INPUT_ELEMENT_DESC Layout[] = {
        {0, 0, 3, SHADER_VALUE_TYPE::FLOAT32, false},
        {1, 0, 2, SHADER_VALUE_TYPE::FLOAT32, false}
    };

    Engine::SHADER_PASS_DESC Pass{};
    Pass.strPassKey = "Default";
    Pass.pVertexShaderSource = DEFAULT_RECT_VS;
    Pass.pPixelShaderSource = DEFAULT_RECT_PS;
    Pass.pInputElements = Layout;
    Pass.iInputElementCount = static_cast<uint32>(std::size(Layout));
    Pass.eCullMode = SHADER_CULL_MODE::NONE;
    Pass.isDepthEnable = false;
    Pass.isBlendEnable = true;
    Pass.isPointSampler = true;

    CShader* pShader = CShader::Create(&Pass, 1, m_pDevice, m_pDeviceContext);
    if (pShader == nullptr)
        return E_FAIL;
    if(FAILED(Add_Resource(L"Shader_VIBuffer_Rect",pShader)))return E_FAIL;

    Engine::SHADER_PASS_DESC WorldPass{};WorldPass.pVertexShaderSource=WORLD_COLOR_VS;WorldPass.pPixelShaderSource=WORLD_COLOR_PS;WorldPass.pInputElements=Layout;WorldPass.iInputElementCount=uint32(std::size(Layout));WorldPass.eCullMode=SHADER_CULL_MODE::NONE;WorldPass.isDepthEnable=true;
    pShader=CShader::Create(&WorldPass,1,m_pDevice,m_pDeviceContext);if(!pShader||FAILED(Add_Resource(L"Shader_Terrain_White",pShader)))return E_FAIL;

    Engine::SHADER_PASS_DESC SkyPass{};SkyPass.pVertexShaderSource=SKY_VS;SkyPass.pPixelShaderSource=SKY_PS;SkyPass.pInputElements=Layout;SkyPass.iInputElementCount=uint32(std::size(Layout));SkyPass.eCullMode=SHADER_CULL_MODE::NONE;SkyPass.isDepthEnable=false;
    pShader=CShader::Create(&SkyPass,1,m_pDevice,m_pDeviceContext);if(!pShader||FAILED(Add_Resource(L"Shader_SkyBox",pShader)))return E_FAIL;
    return S_OK;
}

HRESULT CResourceManager::Ready_Default_Texture()
{
    // 외부 리소스 경로에 의존하지 않는 1x1 흰색 기본 텍스처다.
    constexpr uint32 White = 0xFFFFFFFFu;
    CTexture* pTexture = CTexture::Create_RGBA8(&White, 1, 1, true, m_pDevice, m_pDeviceContext);
    if (pTexture == nullptr)
        return E_FAIL;
    return Add_Resource(L"Texture_Default_White", pTexture);
}

HRESULT CResourceManager::Ready_Default_Material() { return S_OK; }
HRESULT CResourceManager::Ready_Default_Model() { return S_OK; }

HRESULT CResourceManager::Ready_DefaultResource()
{
    if (FAILED(Ready_Default_Shader()) ||
        FAILED(Ready_Default_Texture()) ||
        FAILED(Ready_Default_Material()) ||
        FAILED(Ready_Default_Model()))
        return E_FAIL;
    return S_OK;
}

CResource* CResourceManager::Get_Resource(const std::wstring& strKey,
                                          RESOURCE_TYPE eResourceType,
                                          bool isCopy,
                                          void* pArg) const
{
    CResource* pResource = Find_Resource(strKey, eResourceType);
    if (pResource == nullptr)
        return nullptr;
    if (isCopy)
        return pResource->Copy_Resource(pArg);
    pResource->AddRef();
    return pResource;
}

HRESULT CResourceManager::Add_Resource(const std::wstring& strKey, CResource* pResource)
{
    if (strKey.empty() || pResource == nullptr)
        return E_INVALIDARG;
    const size_t iType = static_cast<size_t>(pResource->Get_ResourceType());
    if (iType >= m_arrayResourceMap.size() || Find_Resource(strKey, pResource->Get_ResourceType()) != nullptr)
        return E_FAIL;

    // 원본과 동일하게 성공 시 전달받은 포인터의 소유권을 ResourceManager가 가져간다.
    m_arrayResourceMap[iType].emplace(strKey, pResource);
    return S_OK;
}

HRESULT CResourceManager::Remove_Resource(const std::wstring& strKey, RESOURCE_TYPE eResourceType)
{
    const size_t iType = static_cast<size_t>(eResourceType);
    if (iType >= m_arrayResourceMap.size())
        return E_INVALIDARG;
    auto& ResourceMap = m_arrayResourceMap[iType];
    const auto Iter = ResourceMap.find(strKey);
    if (Iter == ResourceMap.end())
        return E_FAIL;
    Iter->second->Release();
    ResourceMap.erase(Iter);
    return S_OK;
}

void CResourceManager::Clear_Resource(RESOURCE_TYPE eResourceType)
{
    const size_t iType = static_cast<size_t>(eResourceType);
    if (iType >= m_arrayResourceMap.size())
        return;
    for (auto& Pair : m_arrayResourceMap[iType])
        Pair.second->Release();
    m_arrayResourceMap[iType].clear();
}

CResourceManager* CResourceManager::Create(Diligent::IRenderDevice* pDevice,
                                           Diligent::IDeviceContext* pDeviceContext)
{
    return pDevice != nullptr && pDeviceContext != nullptr
        ? new CResourceManager{pDevice, pDeviceContext}
        : nullptr;
}

void CResourceManager::Free()
{
    for (size_t i = 0; i < m_arrayResourceMap.size(); ++i)
        Clear_Resource(static_cast<RESOURCE_TYPE>(i));
    m_pDeviceContext = nullptr;
    m_pDevice = nullptr;
    CBase::Free();
}

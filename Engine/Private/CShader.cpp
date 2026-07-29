#include "pch.h"
#include "CShader.h"

#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceVariable.h"

namespace
{
Diligent::Uint64 Align16(Diligent::Uint64 iSize)
{
    return (iSize + 15u) & ~Diligent::Uint64{15u};
}

Diligent::VALUE_TYPE ToDiligentValueType(SHADER_VALUE_TYPE eType)
{
    switch (eType)
    {
    case SHADER_VALUE_TYPE::UINT16: return Diligent::VT_UINT16;
    case SHADER_VALUE_TYPE::UINT32: return Diligent::VT_UINT32;
    case SHADER_VALUE_TYPE::FLOAT32:
    default: return Diligent::VT_FLOAT32;
    }
}

Diligent::PRIMITIVE_TOPOLOGY ToDiligentTopology(SHADER_PRIMITIVE_TOPOLOGY eTopology)
{
    switch (eTopology)
    {
    case SHADER_PRIMITIVE_TOPOLOGY::LINE_LIST: return Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
    case SHADER_PRIMITIVE_TOPOLOGY::POINT_LIST: return Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST;
    case SHADER_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST:
    default: return Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

Diligent::CULL_MODE ToDiligentCullMode(SHADER_CULL_MODE eCullMode)
{
    switch (eCullMode)
    {
    case SHADER_CULL_MODE::NONE: return Diligent::CULL_MODE_NONE;
    case SHADER_CULL_MODE::FRONT: return Diligent::CULL_MODE_FRONT;
    case SHADER_CULL_MODE::BACK:
    default: return Diligent::CULL_MODE_BACK;
    }
}

Diligent::TEXTURE_FORMAT ToDiligentTextureFormat(SHADER_TEXTURE_FORMAT eFormat)
{
    switch (eFormat)
    {
    case SHADER_TEXTURE_FORMAT::RGBA8_UNORM: return Diligent::TEX_FORMAT_RGBA8_UNORM;
    case SHADER_TEXTURE_FORMAT::RGBA8_UNORM_SRGB: return Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
    case SHADER_TEXTURE_FORMAT::D32_FLOAT: return Diligent::TEX_FORMAT_D32_FLOAT;
    case SHADER_TEXTURE_FORMAT::UNKNOWN:
    default: return Diligent::TEX_FORMAT_UNKNOWN;
    }
}
}

CShader::CShader(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pDeviceContext)
    : CResource{pDevice, pDeviceContext}
{
    m_eResourceType = RESOURCE_TYPE::SHADER;
}

CShader::CShader(const CShader& Prototype)
    : CResource{Prototype}
{
    m_vecPass.reserve(Prototype.m_vecPass.size());
    for (const PASS& PrototypePass : Prototype.m_vecPass)
    {
        PASS Pass{};
        Pass.strPassKey = PrototypePass.strPassKey;
        Pass.pPipelineState = PrototypePass.pPipelineState;
        if (Pass.pPipelineState != nullptr)
            Pass.pPipelineState->CreateShaderResourceBinding(&Pass.pShaderResourceBinding, true);
        m_vecPass.push_back(std::move(Pass));
    }
}

HRESULT CShader::Initialize(const Engine::SHADER_PASS_DESC* pPassDescs, uint32 iPassCount)
{
    if (m_pDevice == nullptr || pPassDescs == nullptr || iPassCount == 0)
        return E_INVALIDARG;

    m_vecPass.reserve(iPassCount);
    for (uint32 i = 0; i < iPassCount; ++i)
    {
        const Engine::SHADER_PASS_DESC& Desc = pPassDescs[i];
        if (Desc.pVertexShaderSource == nullptr || Desc.pPixelShaderSource == nullptr ||
            (Desc.iInputElementCount > 0 && Desc.pInputElements == nullptr))
            return E_INVALIDARG;

        Diligent::ShaderCreateInfo ShaderCI{};
        ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.EntryPoint = "main";
        // Vulkan 계열에서는 Texture와 Sampler가 결합된 기술자를 사용하므로
        // Diligent의 HLSL 변환기가 두 리소스를 같은 규칙으로 묶도록 한다.
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
        ShaderCI.Desc.Name = "Engine Vertex Shader";
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.Source = Desc.pVertexShaderSource;
        m_pDevice->CreateShader(ShaderCI, &pVS);

        Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
        ShaderCI.Desc.Name = "Engine Pixel Shader";
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.Source = Desc.pPixelShaderSource;
        m_pDevice->CreateShader(ShaderCI, &pPS);
        if (pVS == nullptr || pPS == nullptr)
            return E_FAIL;

        std::vector<Diligent::LayoutElement> LayoutElements;
        LayoutElements.reserve(Desc.iInputElementCount);
        for (uint32 iElement = 0; iElement < Desc.iInputElementCount; ++iElement)
        {
            const Engine::SHADER_INPUT_ELEMENT_DESC& Element = Desc.pInputElements[iElement];
            LayoutElements.emplace_back(
                Element.iInputIndex,
                Element.iBufferSlot,
                Element.iComponentCount,
                ToDiligentValueType(Element.eValueType),
                Element.isNormalized ? Diligent::True : Diligent::False);
        }

        Diligent::GraphicsPipelineStateCreateInfo PipelineCI{};
        PipelineCI.PSODesc.Name = Desc.strPassKey.c_str();
        PipelineCI.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
        // Effect11 변수는 실행 중 Set_GPU_*로 교체할 수 있었다.
        // 같은 사용성을 유지하려면 Diligent 리소스도 기본 Mutable이어야 한다.
        PipelineCI.PSODesc.ResourceLayout.DefaultVariableType =
            Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
        Diligent::SamplerDesc PointSampler{};
        Diligent::ImmutableSamplerDesc ImmutablePointSampler{};
        if (Desc.isPointSampler)
        {
            PointSampler.MinFilter = Diligent::FILTER_TYPE_POINT;
            PointSampler.MagFilter = Diligent::FILTER_TYPE_POINT;
            PointSampler.MipFilter = Diligent::FILTER_TYPE_POINT;
            PointSampler.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
            PointSampler.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
            PointSampler.AddressW = Diligent::TEXTURE_ADDRESS_CLAMP;
            ImmutablePointSampler = {Diligent::SHADER_TYPE_PIXEL, "g_Texture", PointSampler};
            PipelineCI.PSODesc.ResourceLayout.ImmutableSamplers = &ImmutablePointSampler;
            PipelineCI.PSODesc.ResourceLayout.NumImmutableSamplers = 1;
        }
        PipelineCI.GraphicsPipeline.NumRenderTargets = 1;
        PipelineCI.GraphicsPipeline.RTVFormats[0] = ToDiligentTextureFormat(Desc.eRTVFormat);
        PipelineCI.GraphicsPipeline.DSVFormat = ToDiligentTextureFormat(Desc.eDSVFormat);
        PipelineCI.GraphicsPipeline.PrimitiveTopology = ToDiligentTopology(Desc.eTopology);
        PipelineCI.GraphicsPipeline.RasterizerDesc.CullMode = ToDiligentCullMode(Desc.eCullMode);
        PipelineCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = Desc.isDepthEnable ? Diligent::True : Diligent::False;
        if (Desc.isBlendEnable)
        {
            auto& RenderTargetBlend = PipelineCI.GraphicsPipeline.BlendDesc.RenderTargets[0];
            RenderTargetBlend.BlendEnable = Diligent::True;
            RenderTargetBlend.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
            RenderTargetBlend.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        }
        PipelineCI.GraphicsPipeline.InputLayout.LayoutElements = LayoutElements.data();
        PipelineCI.GraphicsPipeline.InputLayout.NumElements = Desc.iInputElementCount;
        PipelineCI.pVS = pVS;
        PipelineCI.pPS = pPS;

        PASS Pass{};
        Pass.strPassKey = Desc.strPassKey;
        m_pDevice->CreateGraphicsPipelineState(PipelineCI, &Pass.pPipelineState);
        if (Pass.pPipelineState == nullptr)
            return E_FAIL;
        Pass.pPipelineState->CreateShaderResourceBinding(&Pass.pShaderResourceBinding, true);
        m_vecPass.push_back(std::move(Pass));
    }
    return S_OK;
}

HRESULT CShader::Copy_Initialize(void* pArg)
{
    return CResource::Copy_Initialize(pArg);
}

CShader::PASS* CShader::Find_Pass(const std::string& strPassKey)
{
    const auto Iter = std::find_if(m_vecPass.begin(), m_vecPass.end(), [&strPassKey](const PASS& Pass) {
        return Pass.strPassKey == strPassKey;
    });
    return Iter != m_vecPass.end() ? &(*Iter) : nullptr;
}

HRESULT CShader::Apply_Pass(const std::string& strPassKey, int32)
{
    PASS* pPass = Find_Pass(strPassKey);
    if (pPass == nullptr || m_pDeviceContext == nullptr)
        return E_FAIL;

    m_pDeviceContext->SetPipelineState(pPass->pPipelineState);
    if (pPass->pShaderResourceBinding != nullptr)
        m_pDeviceContext->CommitShaderResources(
            pPass->pShaderResourceBinding,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    return S_OK;
}

HRESULT CShader::Apply_Pass(uint32 iPassIndex)
{
    if (iPassIndex >= m_vecPass.size())
        return E_INVALIDARG;
    return Apply_Pass(m_vecPass[iPassIndex].strPassKey);
}

HRESULT CShader::Set_GPU_Texture(const std::string& strFindKey, Diligent::ITextureView* pTextureView)
{
    if (pTextureView == nullptr)
        return E_INVALIDARG;

    bool isBound = false;
    for (PASS& Pass : m_vecPass)
    {
        if (Pass.pShaderResourceBinding == nullptr)
            continue;
        Diligent::IShaderResourceVariable* pVariable =
            Pass.pShaderResourceBinding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, strFindKey.c_str());
        if (pVariable != nullptr)
        {
            pVariable->Set(pTextureView);
            isBound = true;
        }
    }
    return isBound ? S_OK : E_FAIL;
}

HRESULT CShader::Set_GPU_Texture(Diligent::ITextureView* pTextureView)
{
    return Set_GPU_Texture("g_Texture", pTextureView);
}

HRESULT CShader::Set_GPU_Textures(uint32 iCountTex, Diligent::ITextureView** ppTextureViews, uint32 iOffset)
{
    if (iCountTex == 0 || ppTextureViews == nullptr)
        return E_INVALIDARG;

    std::vector<Diligent::IDeviceObject*> DeviceObjects;
    DeviceObjects.reserve(iCountTex);
    for (uint32 i = 0; i < iCountTex; ++i)
        DeviceObjects.push_back(ppTextureViews[i]);

    bool isBound = false;
    for (PASS& Pass : m_vecPass)
    {
        if (Pass.pShaderResourceBinding == nullptr)
            continue;
        Diligent::IShaderResourceVariable* pVariable =
            Pass.pShaderResourceBinding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures");
        if (pVariable != nullptr)
        {
            pVariable->SetArray(DeviceObjects.data(), iOffset, iCountTex);
            isBound = true;
        }
    }
    return isBound ? S_OK : E_FAIL;
}

HRESULT CShader::Set_GPU_ConstantBuffer(const std::string& strBufferName,
                                        const void* pData,
                                        uint32 iDataSize,
                                        uint32 iByteOffset)
{
    if (m_pDevice == nullptr || m_pDeviceContext == nullptr || pData == nullptr || iDataSize == 0)
        return E_INVALIDARG;

    const Diligent::Uint64 RequiredSize = Align16(static_cast<Diligent::Uint64>(iByteOffset) + iDataSize);
    auto& pBuffer = m_mapConstantBuffer[strBufferName];
    if (pBuffer == nullptr || pBuffer->GetDesc().Size < RequiredSize)
    {
        Diligent::BufferDesc Desc{};
        Desc.Name = strBufferName.c_str();
        Desc.Size = RequiredSize;
        Desc.Usage = Diligent::USAGE_DEFAULT;
        Desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        m_pDevice->CreateBuffer(Desc, nullptr, &pBuffer);
        if (pBuffer == nullptr)
            return E_FAIL;

        // Effect11은 변수 이름으로 내부 버퍼를 찾아줬지만 Diligent는 HLSL cbuffer 이름에 직접 연결한다.
        for (PASS& Pass : m_vecPass)
        {
            if (Pass.pShaderResourceBinding == nullptr)
                continue;
            for (Diligent::SHADER_TYPE Stage : {Diligent::SHADER_TYPE_VERTEX, Diligent::SHADER_TYPE_PIXEL})
            {
                Diligent::IShaderResourceVariable* pVariable =
                    Pass.pShaderResourceBinding->GetVariableByName(Stage, strBufferName.c_str());
                if (pVariable != nullptr)
                    pVariable->Set(pBuffer);
            }
        }
    }

    m_pDeviceContext->UpdateBuffer(
        pBuffer,
        iByteOffset,
        iDataSize,
        pData,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    return S_OK;
}

std::vector<std::string> CShader::Get_AllPassName() const
{
    std::vector<std::string> Names;
    Names.reserve(m_vecPass.size());
    for (const PASS& Pass : m_vecPass)
        Names.push_back(Pass.strPassKey);
    return Names;
}

CShader* CShader::Create(const Engine::SHADER_PASS_DESC* pPassDescs,
                         uint32 iPassCount,
                         Diligent::IRenderDevice* pDevice,
                         Diligent::IDeviceContext* pDeviceContext)
{
    CShader* pInstance = new CShader{pDevice, pDeviceContext};
    if (FAILED(pInstance->Initialize(pPassDescs, iPassCount)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CResource* CShader::Copy_Resource(void* pArg)
{
    CShader* pCopy = new CShader{*this};
    if (FAILED(pCopy->Copy_Initialize(pArg)))
    {
        pCopy->Release();
        return nullptr;
    }
    return pCopy;
}

void CShader::Free()
{
    m_mapConstantBuffer.clear();
    m_vecPass.clear();
    CResource::Free();
}

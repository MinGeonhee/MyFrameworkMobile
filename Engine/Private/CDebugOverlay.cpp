#include "pch.h"
#include "CDebugOverlay.h"

#include "CShader.h"
#include "CTexture.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"

namespace
{
    constexpr char DEBUG_OVERLAY_VS[] = R"(
struct VSInput { float3 Position : ATTRIB0; float2 Texcoord : ATTRIB1; };
struct PSInput { float4 Position : SV_POSITION; float2 Texcoord : TEXCOORD0; };
void main(in VSInput VSIn, out PSInput PSIn)
{
    PSIn.Position = float4(VSIn.Position, 1.0);
    PSIn.Texcoord = VSIn.Texcoord;
}
)";

    constexpr char DEBUG_OVERLAY_PS[] = R"(
struct PSInput { float4 Position : SV_POSITION; float2 Texcoord : TEXCOORD0; };
Texture2D g_Texture;
SamplerState g_Texture_sampler;
float4 main(in PSInput PSIn) : SV_TARGET
{
    return g_Texture.Sample(g_Texture_sampler, PSIn.Texcoord);
}
)";

    int32 Get_CharacterIndex(char ch)
    {
        constexpr char CHARS[] = "FPS:0123456789";
        for (int32 i = 0; CHARS[i] != '\0'; ++i)
        {
            if (CHARS[i] == ch)
                return i;
        }
        return -1;
    }
}

CDebugOverlay::CDebugOverlay(Device* pDevice, Context* pDeviceContext)
    : m_pDevice{pDevice}
    , m_pDeviceContext{pDeviceContext}
{
}

HRESULT CDebugOverlay::Initialize()
{
    if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
        return E_FAIL;

    constexpr Engine::SHADER_INPUT_ELEMENT_DESC Layout[] = {
        {0, 0, 3, SHADER_VALUE_TYPE::FLOAT32, false},
        {1, 0, 2, SHADER_VALUE_TYPE::FLOAT32, false}
    };

    Engine::SHADER_PASS_DESC Pass{};
    Pass.strPassKey = "Default";
    Pass.pVertexShaderSource = DEBUG_OVERLAY_VS;
    Pass.pPixelShaderSource = DEBUG_OVERLAY_PS;
    Pass.pInputElements = Layout;
    Pass.iInputElementCount = static_cast<uint32>(std::size(Layout));
    Pass.eCullMode = SHADER_CULL_MODE::NONE;
    Pass.isDepthEnable = false;
    Pass.isBlendEnable = true;
    Pass.isPointSampler = true;

    m_pShader = CShader::Create(&Pass, 1, m_pDevice, m_pDeviceContext);
    if (m_pShader == nullptr)
        return E_FAIL;

    constexpr uint32 MAX_VERTEX_COUNT = 1024;
    Diligent::BufferDesc VertexBufferDesc{};
    VertexBufferDesc.Name = "DebugOverlay Vertex Buffer";
    VertexBufferDesc.Size = sizeof(Engine::VTXPOSTEX) * MAX_VERTEX_COUNT;
    VertexBufferDesc.Usage = Diligent::USAGE_DEFAULT;
    VertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    m_pDevice->CreateBuffer(VertexBufferDesc, nullptr, &m_pVertexBuffer);
    if (m_pVertexBuffer == nullptr)
        return E_FAIL;

    m_pNumberFontTexture = CTexture::Create(
        L"../../../Resource/Debug/GeneratedNumberFont.png",
        1,
        m_pDevice,
        m_pDeviceContext);
    if (m_pNumberFontTexture == nullptr)
        return E_FAIL;

    m_Vertices.reserve(MAX_VERTEX_COUNT);

    return S_OK;
}

HRESULT CDebugOverlay::Render_FPS(uint32 iFPS, uint32 iScreenWidth, uint32 iScreenHeight)
{
    if (m_pShader == nullptr || m_pVertexBuffer == nullptr || m_pDeviceContext == nullptr || iScreenWidth == 0 || iScreenHeight == 0)
        return E_FAIL;

    Diligent::Viewport Viewport{0.f, 0.f, static_cast<float>(iScreenWidth), static_cast<float>(iScreenHeight), 0.f, 1.f};
    m_pDeviceContext->SetViewports(1, &Viewport, iScreenWidth, iScreenHeight);
    m_Vertices.clear();

    const float fScale = std::max(1.f, static_cast<float>(std::min(iScreenWidth, iScreenHeight)) / 720.f);
    const float fCharAdvance = 22.f * fScale;
    float fX = 12.f * fScale;
    const float fY = 12.f * fScale;

    const std::string strText = "FPS:" + std::to_string(iFPS);
    for (const char ch : strText)
    {
        if (FAILED(Draw_Character(ch, fX, fY, fScale, iScreenWidth, iScreenHeight)))
            return E_FAIL;
        fX += ch == ':' ? 10.f * fScale : fCharAdvance;
    }

    return Flush();
}

HRESULT CDebugOverlay::Draw_Character(char ch, float fX, float fY, float fScale, uint32 iScreenWidth, uint32 iScreenHeight)
{
    const int32 iCharacterIndex = Get_CharacterIndex(ch);
    if (iCharacterIndex < 0)
        return S_OK;

    constexpr float CHARACTER_COUNT = 14.f;
    const float fU = static_cast<float>(iCharacterIndex) / CHARACTER_COUNT;
    const float fUVWidth = 1.f / CHARACTER_COUNT;

    return Push_Rect(
        fX,
        fY,
        16.f * fScale,
        24.f * fScale,
        iScreenWidth,
        iScreenHeight,
        fU,
        0.f,
        fUVWidth,
        1.f);
}

HRESULT CDebugOverlay::Push_Rect(float fX, float fY, float fWidth, float fHeight, uint32 iScreenWidth, uint32 iScreenHeight,
                                 float fU, float fV, float fUVWidth, float fUVHeight)
{
    if (fWidth <= 0.f || fHeight <= 0.f)
        return S_OK;

    if (m_Vertices.size() + 6 > m_Vertices.capacity())
        return E_FAIL;

    const float fLeft = fX / static_cast<float>(iScreenWidth) * 2.f - 1.f;
    const float fRight = (fX + fWidth) / static_cast<float>(iScreenWidth) * 2.f - 1.f;
    const float fTop = 1.f - fY / static_cast<float>(iScreenHeight) * 2.f;
    const float fBottom = 1.f - (fY + fHeight) / static_cast<float>(iScreenHeight) * 2.f;

    const float fU0 = fU;
    const float fV0 = fV;
    const float fU1 = fU + fUVWidth;
    const float fV1 = fV + fUVHeight;

    m_Vertices.push_back({{fLeft,  fTop,    0.f}, {fU0, fV0}});
    m_Vertices.push_back({{fRight, fTop,    0.f}, {fU1, fV0}});
    m_Vertices.push_back({{fRight, fBottom, 0.f}, {fU1, fV1}});
    m_Vertices.push_back({{fLeft,  fTop,    0.f}, {fU0, fV0}});
    m_Vertices.push_back({{fRight, fBottom, 0.f}, {fU1, fV1}});
    m_Vertices.push_back({{fLeft,  fBottom, 0.f}, {fU0, fV1}});

    return S_OK;
}

HRESULT CDebugOverlay::Flush()
{
    if (m_Vertices.empty())
        return S_OK;
    if (m_pNumberFontTexture == nullptr)
        return E_FAIL;

    const uint32 iVertexDataSize = static_cast<uint32>(sizeof(Engine::VTXPOSTEX) * m_Vertices.size());
    m_pDeviceContext->UpdateBuffer(
        m_pVertexBuffer,
        0,
        iVertexDataSize,
        m_Vertices.data(),
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::IBuffer* pVertexBuffers[] = {m_pVertexBuffer.RawPtr()};
    constexpr Diligent::Uint64 Offsets[] = {0};
    m_pDeviceContext->SetVertexBuffers(
        0,
        1,
        pVertexBuffers,
        Offsets,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);

    if (FAILED(m_pShader->Set_GPU_Texture("g_Texture", m_pNumberFontTexture->GetShaderResourceView(0))) ||
        FAILED(m_pShader->Apply_Pass("Default")))
        return E_FAIL;

    Diligent::DrawAttribs DrawAttribs{};
    DrawAttribs.NumVertices = static_cast<Diligent::Uint32>(m_Vertices.size());
    DrawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->Draw(DrawAttribs);
    return S_OK;
}

CDebugOverlay* CDebugOverlay::Create(Device* pDevice, Context* pDeviceContext)
{
    CDebugOverlay* pOverlay = new CDebugOverlay{pDevice, pDeviceContext};
    if (FAILED(pOverlay->Initialize()))
    {
        pOverlay->Release();
        return nullptr;
    }
    return pOverlay;
}

void CDebugOverlay::Free()
{
    m_pVertexBuffer.Release();
    if (m_pNumberFontTexture != nullptr)
    {
        m_pNumberFontTexture->Release();
        m_pNumberFontTexture = nullptr;
    }
    if (m_pShader != nullptr)
    {
        m_pShader->Release();
        m_pShader = nullptr;
    }
    CBase::Free();
}


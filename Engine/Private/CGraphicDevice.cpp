#include "CGraphicDevice.h"
#include "CShader.h"

#if defined(_WIN32)
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Platforms/Win32/interface/Win32NativeWindow.h"
#elif defined(__ANDROID__)
#include <android/native_window.h>
#include "Platforms/Android/interface/AndroidNativeWindow.h"
#endif

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"

CGraphicDevice* CGraphicDevice::s_pInstance = nullptr;

namespace
{
    /* 엔진은 원본처럼 고정 해상도(예 : 1280 x 720)의 백버퍼에 그리고 , Present 직전에 실제 화면(SwapChain)에 맞춰 늘려서 출력한다. */
    /* 원본 셰이더 / UI 좌표가 1280 x 720 을 기준으로 작성되어 있기 때문에 Android 의 다양한 화면 크기에서도 그대로 동작하게 하기 위함이다. */
    constexpr const char* g_pBlitShaderSource = R"(
Texture2D    g_BackBuffer;
SamplerState g_BackBuffer_sampler;

struct VS_OUT
{
    float4 vPosition : SV_Position;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VSMain(uint iVertexID : SV_VertexID)
{
    VS_OUT Out;
    float2 vUV = float2((iVertexID << 1) & 2, iVertexID & 2);
    Out.vPosition = float4(vUV * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);
    Out.vTexcoord = vUV;
    return Out;
}

float4 PSMain(VS_OUT In) : SV_Target
{
    return g_BackBuffer.Sample(g_BackBuffer_sampler, In.vTexcoord);
}

float4 PSClear(VS_OUT In) : SV_Target
{
    return float4(0.f, 0.f, 0.f, 0.f);
}
)";

    struct BLIT_RESOURCES
    {
        ComPtr<Diligent::ITexture>                  pBackBufferTexture;
        ComPtr<Diligent::ITexture>                  pDepthTexture;
        ComPtr<Diligent::IPipelineState>            pBlitPSO;
        ComPtr<Diligent::IShaderResourceBinding>    pBlitSRB;
    };
    BLIT_RESOURCES g_tBlit;

    /* Diligent 메시지 (셰이더 컴파일 오류 , 검증 오류 등) 를 엔진 로그로 보낸다 */
    void DILIGENT_CALL_TYPE Diligent_MessageCallback(Diligent::DEBUG_MESSAGE_SEVERITY eSeverity, const Diligent::Char* pMessage, const Diligent::Char* pFunction, const Diligent::Char* pFile, int iLine)
    {
        static const char* s_pSeverity[] = { "Info", "Warning", "Error", "Fatal" };
        const int iIndex = static_cast<int>(eSeverity);
        Engine::Engine_Log("[Diligent %s] %s", (iIndex >= 0 && iIndex < 4) ? s_pSeverity[iIndex] : "?", pMessage != nullptr ? pMessage : "");
    }
}

CGraphicDevice::CGraphicDevice()
    : m_pDevice{ nullptr }
    , m_pDeviceContext{ nullptr }
    , m_iWinSizeX{}, m_iWinSizeY{}
    , m_hInst(nullptr) , m_hWnd(nullptr)
    , m_tOriginViewPort{}
    , m_eGraphicAPI(GRAPHIC_API::D3D11)
    , m_iFrameIndex(0)
    , m_tState{}
    , m_pCurRTVs{}
    , m_pCurDSV(nullptr)
    , m_iRTWidth(0), m_iRTHeight(0)
    , m_tCurViewPort{}
    , m_isScissorSet(false)
    , m_tCurScissor{}
    , m_pCurVertexBuffers{}
    , m_iCurVertexOffsets{}
    , m_pCurIndexBuffer(nullptr)
    , m_eCurIndexType(Diligent::VT_UINT32)
    , m_pCurShader(nullptr)
    , m_iCurPass(0)
    , m_pBoundPSO(nullptr)
{
    /* D3D11 디바이스 기본 상태와 같게 맞춘다 (Cull Back , Depth Less , Blend Off) */
    m_tState.tRasterizer.FillMode = Diligent::FILL_MODE_SOLID;
    m_tState.tRasterizer.CullMode = Diligent::CULL_MODE_BACK;
    m_tState.tRasterizer.FrontCounterClockwise = false;
    m_tState.tRasterizer.DepthClipEnable = true;
    m_tState.tDepthStencil.DepthEnable = true;
    m_tState.tDepthStencil.DepthWriteEnable = true;
    m_tState.tDepthStencil.DepthFunc = Diligent::COMPARISON_FUNC_LESS;
}

HRESULT CGraphicDevice::Initialize(const GraphicDevice_Desc& tDesc, ComPtr<Device>* pDevice, ComPtr<DeviceContext>* pContext)
{
    if (tDesc.hWnd == nullptr)
        return E_FAIL;

    m_eGraphicAPI = tDesc.eGraphicAPI;
#if !defined(_WIN32)
    /* Android 는 Vulkan 만 지원 */
    m_eGraphicAPI = GRAPHIC_API::VULKAN;
#endif

    HRESULT hr = E_FAIL;
#if defined(_WIN32)
    if (m_eGraphicAPI == GRAPHIC_API::D3D11)
        hr = Initialize_DirectX11(tDesc);
    else
#endif
        hr = Initialize_Vulkan(tDesc);

    if (FAILED(hr))
        return E_FAIL;

    s_pInstance = this;

    m_iWinSizeX = tDesc.iWinSizeX;
    m_iWinSizeY = tDesc.iWinSizeY;

    m_tOriginViewPort.TopLeftX = 0.f;
    m_tOriginViewPort.TopLeftY = 0.f;
    m_tOriginViewPort.Width = (float)m_iWinSizeX;
    m_tOriginViewPort.Height = (float)m_iWinSizeY;
    m_tOriginViewPort.MaxDepth = 1.f;
    m_tOriginViewPort.MinDepth = 0.f;

    /* ---- 엔진 백버퍼 (원본 SwapChain 백버퍼 + DepthStencil 역할) ---- */
    Diligent::TextureDesc tTexDesc{};
    tTexDesc.Name = "Engine BackBuffer";
    tTexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    tTexDesc.Width = m_iWinSizeX;
    tTexDesc.Height = m_iWinSizeY;
    tTexDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    tTexDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
    tTexDesc.ClearValue.Format = tTexDesc.Format;
    m_pDevice->CreateTexture(tTexDesc, nullptr, &g_tBlit.pBackBufferTexture);
    if (g_tBlit.pBackBufferTexture == nullptr)
        return E_FAIL;

    Diligent::TextureDesc tDepthDesc{};
    tDepthDesc.Name = "Engine DepthStencil";
    tDepthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    tDepthDesc.Width = m_iWinSizeX;
    tDepthDesc.Height = m_iWinSizeY;
    tDepthDesc.Format = Is_TextureFormatSupported(Diligent::TEX_FORMAT_D24_UNORM_S8_UINT) ? Diligent::TEX_FORMAT_D24_UNORM_S8_UINT : Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;
    tDepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
    tDepthDesc.ClearValue.Format = tDepthDesc.Format;
    tDepthDesc.ClearValue.DepthStencil.Depth = 1.f;
    m_pDevice->CreateTexture(tDepthDesc, nullptr, &g_tBlit.pDepthTexture);
    if (g_tBlit.pDepthTexture == nullptr)
        return E_FAIL;

    /* ---- 백버퍼 → SwapChain 출력용 PSO ---- */
    {
        Diligent::ShaderCreateInfo tShaderCI{};
        tShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        tShaderCI.Desc.UseCombinedTextureSamplers = true;
        tShaderCI.Source = g_pBlitShaderSource;

        ComPtr<Diligent::IShader> pVS, pPS;
        tShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        tShaderCI.Desc.Name = "Engine Blit VS";
        tShaderCI.EntryPoint = "VSMain";
        m_pDevice->CreateShader(tShaderCI, &pVS);
        tShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        tShaderCI.Desc.Name = "Engine Blit PS";
        tShaderCI.EntryPoint = "PSMain";
        m_pDevice->CreateShader(tShaderCI, &pPS);
        if (pVS == nullptr || pPS == nullptr)
            return E_FAIL;

        Diligent::GraphicsPipelineStateCreateInfo tPSOCI{};
        tPSOCI.PSODesc.Name = "Engine Blit PSO";
        tPSOCI.GraphicsPipeline.NumRenderTargets = 1;
        tPSOCI.GraphicsPipeline.RTVFormats[0] = m_pSwapChain->GetDesc().ColorBufferFormat;
        tPSOCI.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
        tPSOCI.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        tPSOCI.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
        tPSOCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
        tPSOCI.pVS = pVS.Get();
        tPSOCI.pPS = pPS.Get();

        Diligent::SamplerDesc tLinearClamp{};
        tLinearClamp.MinFilter = Diligent::FILTER_TYPE_LINEAR;
        tLinearClamp.MagFilter = Diligent::FILTER_TYPE_LINEAR;
        tLinearClamp.MipFilter = Diligent::FILTER_TYPE_LINEAR;
        tLinearClamp.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
        tLinearClamp.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
        tLinearClamp.AddressW = Diligent::TEXTURE_ADDRESS_CLAMP;
        Diligent::ImmutableSamplerDesc tImmutableSampler{ Diligent::SHADER_TYPE_PIXEL, "g_BackBuffer", tLinearClamp };
        Diligent::ShaderResourceVariableDesc tVariable{ Diligent::SHADER_TYPE_PIXEL, "g_BackBuffer", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE };
        tPSOCI.PSODesc.ResourceLayout.ImmutableSamplers = &tImmutableSampler;
        tPSOCI.PSODesc.ResourceLayout.NumImmutableSamplers = 1;
        tPSOCI.PSODesc.ResourceLayout.Variables = &tVariable;
        tPSOCI.PSODesc.ResourceLayout.NumVariables = 1;

        m_pDevice->CreateGraphicsPipelineState(tPSOCI, &g_tBlit.pBlitPSO);
        if (g_tBlit.pBlitPSO == nullptr)
            return E_FAIL;
        g_tBlit.pBlitPSO->CreateShaderResourceBinding(&g_tBlit.pBlitSRB, true);
        g_tBlit.pBlitSRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_BackBuffer")->Set(
            g_tBlit.pBackBufferTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    }

    /* 장치에 바인드해놓을 렌더 타겟들과 뎁스스텐실뷰를 세팅한다. */
    ShaderResourceView* pRTVs[] = { GetRTV().Get() };
    OM_SetRenderTargets(1, pRTVs, GetDSV().Get());
    RS_SetViewports(1, &m_tOriginViewPort);

    *pDevice = m_pDevice;
    *pContext = m_pDeviceContext;

    m_hWnd = tDesc.hWnd;
    m_hInst = tDesc.hInst;

    return S_OK;
}

HRESULT CGraphicDevice::Initialize_Vulkan(const GraphicDevice_Desc& tDesc)
{
    Diligent::IEngineFactoryVk* pEngineFactory = Diligent::LoadAndGetEngineFactoryVk();
    if (pEngineFactory == nullptr)
        return E_FAIL;

    pEngineFactory->SetBreakOnError(false);
    pEngineFactory->SetMessageCallback(Diligent_MessageCallback);

    Diligent::EngineVkCreateInfo tEngineCI{};
#if defined(_DEBUG)
    tEngineCI.EnableValidation = true;
#endif
    Diligent::IRenderDevice* pRawDevice = nullptr;
    Diligent::IDeviceContext* pRawContext = nullptr;
    pEngineFactory->CreateDeviceAndContextsVk(tEngineCI, &pRawDevice, &pRawContext);
    if (pRawDevice == nullptr || pRawContext == nullptr)
        return E_FAIL;
    m_pDevice.Attach(pRawDevice);
    m_pDeviceContext.Attach(pRawContext);

    Diligent::SwapChainDesc tSwapChainDesc{};
    tSwapChainDesc.Width = tDesc.iWinSizeX;
    tSwapChainDesc.Height = tDesc.iWinSizeY;
    tSwapChainDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    tSwapChainDesc.DepthBufferFormat = Diligent::TEX_FORMAT_UNKNOWN;

#if defined(__ANDROID__)
    /* 실제 Surface 크기는 Diligent 가 ANativeWindow 에서 조회한다. */
    tSwapChainDesc.Width = 0;
    tSwapChainDesc.Height = 0;
    tSwapChainDesc.PreTransform = Diligent::SURFACE_TRANSFORM_IDENTITY;

    Diligent::AndroidNativeWindow tNativeWindow{};
    tNativeWindow.pAWindow = static_cast<ANativeWindow*>(tDesc.hWnd);
#else
    const Diligent::Win32NativeWindow tNativeWindow{ tDesc.hWnd };
#endif

    Diligent::ISwapChain* pRawSwapChain = nullptr;
    pEngineFactory->CreateSwapChainVk(m_pDevice.Get(), m_pDeviceContext.Get(), tSwapChainDesc, tNativeWindow, &pRawSwapChain);
    if (pRawSwapChain == nullptr)
        return E_FAIL;
    m_pSwapChain.Attach(pRawSwapChain);

    return S_OK;
}

#if defined(_WIN32)
HRESULT CGraphicDevice::Initialize_DirectX11(const GraphicDevice_Desc& tDesc)
{
    Diligent::IEngineFactoryD3D11* pEngineFactory = Diligent::LoadAndGetEngineFactoryD3D11();
    if (pEngineFactory == nullptr)
        return E_FAIL;

    pEngineFactory->SetBreakOnError(false);
    pEngineFactory->SetMessageCallback(Diligent_MessageCallback);

    Diligent::EngineD3D11CreateInfo tEngineCI{};
#if defined(_DEBUG)
    tEngineCI.SetValidationLevel(Diligent::VALIDATION_LEVEL_1);
#endif
    Diligent::IRenderDevice* pRawDevice = nullptr;
    Diligent::IDeviceContext* pRawContext = nullptr;
    pEngineFactory->CreateDeviceAndContextsD3D11(tEngineCI, &pRawDevice, &pRawContext);
    if (pRawDevice == nullptr || pRawContext == nullptr)
        return E_FAIL;
    m_pDevice.Attach(pRawDevice);
    m_pDeviceContext.Attach(pRawContext);

    Diligent::SwapChainDesc tSwapChainDesc{};
    tSwapChainDesc.Width = tDesc.iWinSizeX;
    tSwapChainDesc.Height = tDesc.iWinSizeY;
    tSwapChainDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM;
    tSwapChainDesc.DepthBufferFormat = Diligent::TEX_FORMAT_UNKNOWN;

    const Diligent::Win32NativeWindow tNativeWindow{ tDesc.hWnd };
    Diligent::ISwapChain* pRawSwapChain = nullptr;
    pEngineFactory->CreateSwapChainD3D11(m_pDevice.Get(), m_pDeviceContext.Get(), tSwapChainDesc, Diligent::FullScreenModeDesc{}, tNativeWindow, &pRawSwapChain);
    if (pRawSwapChain == nullptr)
        return E_FAIL;
    m_pSwapChain.Attach(pRawSwapChain);

    return S_OK;
}
#endif

ComPtr<RTV> CGraphicDevice::GetRTV()
{
    if (g_tBlit.pBackBufferTexture == nullptr)
        return nullptr;
    return g_tBlit.pBackBufferTexture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
}

ComPtr<DepthStencilView> CGraphicDevice::GetDSV()
{
    if (g_tBlit.pDepthTexture == nullptr)
        return nullptr;
    return g_tBlit.pDepthTexture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
}

Texture2D* CGraphicDevice::Get_BackBuffer()
{
    return g_tBlit.pBackBufferTexture.Get();
}

HRESULT CGraphicDevice::Clear_BackBuffer_View(const Float4* pClearColor)
{
    if (nullptr == m_pDeviceContext)
        return E_FAIL;

    ShaderResourceView* pRTVs[] = { GetRTV().Get() };
    OM_SetRenderTargets(1, pRTVs, GetDSV().Get());

    /* 백버퍼를 초기화한다.  */
    m_pDeviceContext->ClearRenderTarget(pRTVs[0], reinterpret_cast<const float*>(pClearColor), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return S_OK;
}

HRESULT CGraphicDevice::Clear_DepthStencil_View()
{
    if (nullptr == m_pDeviceContext)
        return E_FAIL;

    m_pDeviceContext->ClearDepthStencil(GetDSV().Get(), Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return S_OK;
}

HRESULT CGraphicDevice::Present()
{
    if (nullptr == m_pSwapChain)
        return E_FAIL;

    /* ---- 엔진 백버퍼를 SwapChain 백버퍼에 화면비를 유지하며 출력한다 ---- */
    Diligent::ITextureView* pSwapRTV = m_pSwapChain->GetCurrentBackBufferRTV();
    const Diligent::SwapChainDesc& tSCDesc = m_pSwapChain->GetDesc();

    m_pDeviceContext->SetRenderTargets(1, &pSwapRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    const float vBlack[4] = { 0.f, 0.f, 0.f, 1.f };
    m_pDeviceContext->ClearRenderTarget(pSwapRTV, vBlack, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    const float fScale = (std::min)(tSCDesc.Width / (float)m_iWinSizeX, tSCDesc.Height / (float)m_iWinSizeY);
    Diligent::Viewport tVP{};
    tVP.Width = m_iWinSizeX * fScale;
    tVP.Height = m_iWinSizeY * fScale;
    tVP.TopLeftX = (tSCDesc.Width - tVP.Width) * 0.5f;
    tVP.TopLeftY = (tSCDesc.Height - tVP.Height) * 0.5f;
    m_pDeviceContext->SetViewports(1, &tVP, tSCDesc.Width, tSCDesc.Height);

    m_pDeviceContext->SetPipelineState(g_tBlit.pBlitPSO.Get());
    m_pDeviceContext->CommitShaderResources(g_tBlit.pBlitSRB.Get(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::DrawAttribs tDraw{ 3, Diligent::DRAW_FLAG_VERIFY_ALL };
    m_pDeviceContext->Draw(tDraw);
    m_pBoundPSO = nullptr;

    m_pSwapChain->Present(1);
    ++m_iFrameIndex;

    /* 다음 프레임은 다시 엔진 백버퍼에 그린다 */
    ShaderResourceView* pRTVs[] = { GetRTV().Get() };
    OM_SetRenderTargets(1, pRTVs, GetDSV().Get());

    return S_OK;
}

void CGraphicDevice::Resize(uint32 iWinSizeX, uint32 iWinSizeY)
{
    if (m_pSwapChain == nullptr || iWinSizeX == 0 || iWinSizeY == 0)
        return;

    const Diligent::SwapChainDesc& tSCDesc = m_pSwapChain->GetDesc();
    if (tSCDesc.Width == iWinSizeX && tSCDesc.Height == iWinSizeY)
        return;

#if defined(__ANDROID__)
    m_pSwapChain->Resize(iWinSizeX, iWinSizeY, Diligent::SURFACE_TRANSFORM_IDENTITY);
#else
    m_pSwapChain->Resize(iWinSizeX, iWinSizeY);
#endif
}

void CGraphicDevice::Set_ScissorRectsOrigin()
{
    RECT_DESC tRect;
    tRect.left = (LONG)m_tOriginViewPort.TopLeftX;
    tRect.top = (LONG)m_tOriginViewPort.TopLeftY;
    tRect.right = (LONG)tRect.left + (LONG)m_tOriginViewPort.Width;
    tRect.bottom = (LONG)tRect.top + (LONG)m_tOriginViewPort.Height;

    RS_SetScissorRects(1, &tRect);
}

HRESULT CGraphicDevice::Set_GPU_OriginViewPort()
{
    RS_SetViewports(1, &m_tOriginViewPort);
    return S_OK;
}

#pragma region DX11 State Machine

void CGraphicDevice::OM_SetRenderTargets(uint32 iNumRTV, ShaderResourceView* const* ppRTVs, DepthStencilView* pDSV)
{
    /* D3D11 은 8개 슬롯에 nullptr 를 넣어 뒤를 비울 수 있으므로 실제 개수만 센다 */
    uint32 iCount = 0;
    ShaderResourceView* pValidRTVs[8]{};
    for (uint32 i = 0; i < iNumRTV && i < 8; ++i)
    {
        if (ppRTVs != nullptr && ppRTVs[i] != nullptr)
            pValidRTVs[iCount++] = ppRTVs[i];
    }

    for (uint32 i = 0; i < 8; ++i)
        m_pCurRTVs[i] = i < iCount ? pValidRTVs[i] : nullptr;
    m_pCurDSV = pDSV;

    m_tState.iNumRenderTargets = iCount;
    for (uint32 i = 0; i < 8; ++i)
        m_tState.eRTVFormats[i] = i < iCount ? pValidRTVs[i]->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;
    m_tState.eDSVFormat = pDSV != nullptr ? pDSV->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    m_pDeviceContext->SetRenderTargets(iCount, pValidRTVs, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Update_RenderTargetSize();
    /* Diligent 는 SetRenderTargets 시 Viewport 를 RT 전체로 되돌리므로 D3D11 처럼 기존 값을 다시 적용한다 */
    Apply_ViewportAndScissor();
}

void CGraphicDevice::OM_GetRenderTargets(uint32 iNumRTV, RenderTargetView** ppRTVs, DepthStencilView** ppDSV) const
{
    /* D3D11 OMGetRenderTargets 와 같이 반환한 View 의 참조 카운트를 올린다 (사용 후 Release 필요) */
    for (uint32 i = 0; i < iNumRTV && ppRTVs != nullptr; ++i)
    {
        ppRTVs[i] = i < 8 ? m_pCurRTVs[i] : nullptr;
        if (ppRTVs[i] != nullptr)
            ppRTVs[i]->AddRef();
    }
    if (ppDSV != nullptr)
    {
        *ppDSV = m_pCurDSV;
        if (*ppDSV != nullptr)
            (*ppDSV)->AddRef();
    }
}

void CGraphicDevice::OM_SetBlendState(const BLEND_DESC& tDesc, const float* pBlendFactor)
{
    m_tState.tBlend = tDesc;
    for (int i = 0; i < 4; ++i)
        m_tState.fBlendFactor[i] = pBlendFactor != nullptr ? pBlendFactor[i] : 1.f;
}

void CGraphicDevice::OM_SetDepthStencilState(const DEPTH_STENCIL_DESC& tDesc, uint32 iStencilRef)
{
    m_tState.tDepthStencil = tDesc;
    m_tState.iStencilRef = iStencilRef;
}

void CGraphicDevice::RS_SetViewports(uint32 iNumViewports, const VIEWPORT_DESC* pViewports)
{
    if (iNumViewports == 0 || pViewports == nullptr)
        return;

    m_tCurViewPort = pViewports[0];
    Apply_ViewportAndScissor();
}

void CGraphicDevice::RS_GetViewports(uint32* pNumViewports, VIEWPORT_DESC* pViewports) const
{
    if (pNumViewports != nullptr)
        *pNumViewports = 1;
    if (pViewports != nullptr)
        *pViewports = m_tCurViewPort;
}

void CGraphicDevice::RS_SetScissorRects(uint32 iNumRects, const RECT_DESC* pRects)
{
    if (iNumRects == 0 || pRects == nullptr)
        m_isScissorSet = false;
    else
    {
        m_isScissorSet = true;
        m_tCurScissor = pRects[0];
    }
    Apply_ViewportAndScissor();
}

void CGraphicDevice::RS_SetState(const RASTERIZER_DESC& tDesc)
{
    m_tState.tRasterizer = tDesc;
}

void CGraphicDevice::IA_SetVertexBuffers(uint32 iStartSlot, uint32 iNumBuffers, Buffer* const* ppBuffers, const uint32* pStrides, const uint64* pOffsets)
{
    for (uint32 i = 0; i < iNumBuffers; ++i)
    {
        const uint32 iSlot = iStartSlot + i;
        if (iSlot >= 2)
            break;
        m_pCurVertexBuffers[iSlot] = ppBuffers != nullptr ? ppBuffers[i] : nullptr;
        m_iCurVertexOffsets[iSlot] = pOffsets != nullptr ? pOffsets[i] : 0;
        m_tState.iVertexStrides[iSlot] = pStrides != nullptr ? pStrides[i] : 0;
    }

    uint32 iBound = m_pCurVertexBuffers[1] != nullptr ? 2u : (m_pCurVertexBuffers[0] != nullptr ? 1u : 0u);
    if (iBound == 0)
        return;

    m_pDeviceContext->SetVertexBuffers(0, iBound, m_pCurVertexBuffers, m_iCurVertexOffsets,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
}

void CGraphicDevice::IA_SetIndexBuffer(Buffer* pIndexBuffer, Diligent::VALUE_TYPE eIndexType, uint64 iOffset)
{
    m_pCurIndexBuffer = pIndexBuffer;
    m_eCurIndexType = eIndexType;
    if (pIndexBuffer != nullptr)
        m_pDeviceContext->SetIndexBuffer(pIndexBuffer, iOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void CGraphicDevice::IA_SetPrimitiveTopology(PRIMITIVE_TOPOLOGY eTopology)
{
    m_tState.eTopology = eTopology;
}

void CGraphicDevice::Set_CurrentPass(CShader* pShader, uint32 iPassIndex)
{
    m_pCurShader = pShader;
    m_iCurPass = iPassIndex;
}

HRESULT CGraphicDevice::Flush_Draw()
{
    if (m_pCurShader == nullptr)
        return E_FAIL;

    Diligent::IPipelineState* pPSO = nullptr;
    Diligent::IShaderResourceBinding* pSRB = nullptr;
    if (FAILED(m_pCurShader->Prepare_Draw(m_iCurPass, m_tState, &pPSO, &pSRB)) || pPSO == nullptr)
        return E_FAIL;

    if (pPSO != m_pBoundPSO)
    {
        m_pDeviceContext->SetPipelineState(pPSO);
        m_pBoundPSO = pPSO;
    }
    m_pDeviceContext->SetStencilRef(m_tState.iStencilRef);
    m_pDeviceContext->SetBlendFactors(m_tState.fBlendFactor);

    if (pSRB != nullptr)
        m_pDeviceContext->CommitShaderResources(pSRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    return S_OK;
}

HRESULT CGraphicDevice::Draw(uint32 iVertexCount, uint32 iStartVertex)
{
    if (FAILED(Flush_Draw()))
        return E_FAIL;

    Diligent::DrawAttribs tAttribs{};
    tAttribs.NumVertices = iVertexCount;
    tAttribs.StartVertexLocation = iStartVertex;
    tAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->Draw(tAttribs);
    return S_OK;
}

HRESULT CGraphicDevice::DrawIndexed(uint32 iIndexCount, uint32 iStartIndex, int32 iBaseVertex)
{
    if (FAILED(Flush_Draw()))
        return E_FAIL;

    Diligent::DrawIndexedAttribs tAttribs{};
    tAttribs.NumIndices = iIndexCount;
    tAttribs.IndexType = m_eCurIndexType;
    tAttribs.FirstIndexLocation = iStartIndex;
    tAttribs.BaseVertex = iBaseVertex;
    tAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->DrawIndexed(tAttribs);
    return S_OK;
}

HRESULT CGraphicDevice::DrawInstanced(uint32 iVertexCountPerInstance, uint32 iInstanceCount, uint32 iStartVertex, uint32 iStartInstance)
{
    if (FAILED(Flush_Draw()))
        return E_FAIL;

    Diligent::DrawAttribs tAttribs{};
    tAttribs.NumVertices = iVertexCountPerInstance;
    tAttribs.NumInstances = iInstanceCount;
    tAttribs.StartVertexLocation = iStartVertex;
    tAttribs.FirstInstanceLocation = iStartInstance;
    tAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->Draw(tAttribs);
    return S_OK;
}

HRESULT CGraphicDevice::DrawIndexedInstanced(uint32 iIndexCountPerInstance, uint32 iInstanceCount, uint32 iStartIndex, int32 iBaseVertex, uint32 iStartInstance)
{
    if (FAILED(Flush_Draw()))
        return E_FAIL;

    Diligent::DrawIndexedAttribs tAttribs{};
    tAttribs.NumIndices = iIndexCountPerInstance;
    tAttribs.NumInstances = iInstanceCount;
    tAttribs.IndexType = m_eCurIndexType;
    tAttribs.FirstIndexLocation = iStartIndex;
    tAttribs.BaseVertex = iBaseVertex;
    tAttribs.FirstInstanceLocation = iStartInstance;
    tAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->DrawIndexed(tAttribs);
    return S_OK;
}

HRESULT CGraphicDevice::Clear_RenderTarget_Rect(ShaderResourceView* pRTV, const Color& vColor, const RECT_DESC& tRect)
{
    /* Diligent 는 영역 Clear(ClearView) 가 없으므로 전체를 지운다. (A-plan 측정 모드 전용) */
    if (pRTV == nullptr)
        return E_FAIL;

    m_pDeviceContext->ClearRenderTarget(pRTV, &vColor.x, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    return S_OK;
}

void CGraphicDevice::Invalidate_BoundState()
{
    m_pBoundPSO = nullptr;

    /* 정점버퍼를 엔진 상태로 되돌린다 */
    uint32 iBound = m_pCurVertexBuffers[1] != nullptr ? 2u : (m_pCurVertexBuffers[0] != nullptr ? 1u : 0u);
    if (iBound > 0)
        m_pDeviceContext->SetVertexBuffers(0, iBound, m_pCurVertexBuffers, m_iCurVertexOffsets,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    if (m_pCurIndexBuffer != nullptr)
        m_pDeviceContext->SetIndexBuffer(m_pCurIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Apply_ViewportAndScissor();
}

TEXTURE_FORMAT CGraphicDevice::Get_SupportedRenderTargetFormat(TEXTURE_FORMAT eFormat) const
{
    auto Is_RenderTargetFormat = [this](TEXTURE_FORMAT eFmt) {
        const Diligent::TextureFormatInfoExt& tInfo = m_pDevice->GetTextureFormatInfoExt(eFmt);
        return tInfo.Supported && (tInfo.BindFlags & Diligent::BIND_RENDER_TARGET) != 0;
    };

    if (Is_RenderTargetFormat(eFormat))
        return eFormat;

    /* Vulkan 에서 RGBA16_UNORM 은 Render Target 필수 지원 형식이 아니다 → RGBA16_FLOAT */
    switch (eFormat)
    {
    case Diligent::TEX_FORMAT_RGBA16_UNORM:
    case Diligent::TEX_FORMAT_RGBA16_SNORM:
        if (Is_RenderTargetFormat(Diligent::TEX_FORMAT_RGBA16_FLOAT)) return Diligent::TEX_FORMAT_RGBA16_FLOAT;
        break;
    case Diligent::TEX_FORMAT_RGBA32_FLOAT:
        if (Is_RenderTargetFormat(Diligent::TEX_FORMAT_RGBA16_FLOAT)) return Diligent::TEX_FORMAT_RGBA16_FLOAT;
        break;
    default:
        break;
    }
    return Diligent::TEX_FORMAT_RGBA8_UNORM;
}

bool CGraphicDevice::Is_TextureFormatSupported(TEXTURE_FORMAT eFormat) const
{
    if (m_pDevice == nullptr)
        return false;
    const Diligent::TextureFormatInfoExt& tInfo = m_pDevice->GetTextureFormatInfoExt(eFormat);
    if (!tInfo.Supported)
        return false;
    if (eFormat == Diligent::TEX_FORMAT_D24_UNORM_S8_UINT || eFormat == Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT || eFormat == Diligent::TEX_FORMAT_D32_FLOAT)
        return (tInfo.BindFlags & Diligent::BIND_DEPTH_STENCIL) != 0;
    return (tInfo.BindFlags & Diligent::BIND_SHADER_RESOURCE) != 0;
}

void CGraphicDevice::Update_RenderTargetSize()
{
    Diligent::ITextureView* pView = m_pCurRTVs[0] != nullptr ? m_pCurRTVs[0] : m_pCurDSV;
    if (pView == nullptr)
    {
        m_iRTWidth = m_iWinSizeX;
        m_iRTHeight = m_iWinSizeY;
        return;
    }
    const Diligent::TextureDesc& tDesc = pView->GetTexture()->GetDesc();
    m_iRTWidth = tDesc.Width;
    m_iRTHeight = tDesc.Height;
}

void CGraphicDevice::Apply_ViewportAndScissor()
{
    if (m_pDeviceContext == nullptr || m_iRTWidth == 0 || m_iRTHeight == 0)
        return;

    VIEWPORT_DESC tViewPort = m_tCurViewPort;
    if (tViewPort.Width <= 0.f || tViewPort.Height <= 0.f)
        tViewPort = m_tOriginViewPort;
    m_pDeviceContext->SetViewports(1, &tViewPort, m_iRTWidth, m_iRTHeight);

    RECT_DESC tScissor{ 0, 0, static_cast<Diligent::Int32>(m_iRTWidth), static_cast<Diligent::Int32>(m_iRTHeight) };
    if (m_isScissorSet)
        tScissor = m_tCurScissor;
    m_pDeviceContext->SetScissorRects(1, &tScissor, m_iRTWidth, m_iRTHeight);
}

#pragma endregion

CGraphicDevice* CGraphicDevice::Create( CONST GraphicDevice_Desc& tDesc, ComPtr<Device>* pDevice, ComPtr<DeviceContext>* pDeviceContextOut)
{
    CGraphicDevice* pInstance = new CGraphicDevice();

    if (FAILED(pInstance->Initialize(tDesc,pDevice, pDeviceContextOut)))
    {
        MSG_BOX("Failed to Created : CGraphicDevice");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGraphicDevice::Free()
{
    __super::Free();

    if (m_pDeviceContext != nullptr)
    {
        m_pDeviceContext->Flush();
        m_pDeviceContext->WaitForIdle();
    }

    g_tBlit.pBlitSRB.Reset();
    g_tBlit.pBlitPSO.Reset();
    g_tBlit.pBackBufferTexture.Reset();
    g_tBlit.pDepthTexture.Reset();

    m_pSwapChain.Reset();
    m_pDeviceContext.Reset();
    m_pDevice.Reset();

    if (s_pInstance == this)
        s_pInstance = nullptr;
}

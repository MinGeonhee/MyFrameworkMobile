#include "pch.h"
#include "CGraphicDevice.h"
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Platforms/Win32/interface/Win32NativeWindow.h"

HRESULT CGraphicDevice::Initialize(const Engine_Desc& tEngineDesc)
{
    if (tEngineDesc.pNativeWindow == nullptr)
        return E_FAIL;

    m_tEngineDesc = tEngineDesc;

    switch (m_tEngineDesc.eGraphicAPI)
    {
    case GRAPHIC_API::D3D11:
        return Initialize_DirectX11();

    case GRAPHIC_API::VULKAN:
        return Initialize_Vulkan();

    default:
        return E_FAIL;
    }
}

HRESULT CGraphicDevice::Initialize_Vulkan()
{
    Diligent::IEngineFactoryVk* pEngineFactory = Diligent::LoadAndGetEngineFactoryVk();
    if (pEngineFactory == nullptr)
        return E_FAIL;

    pEngineFactory->SetBreakOnError(false);

    Diligent::EngineVkCreateInfo EngineCreateInfo{};
    pEngineFactory->CreateDeviceAndContextsVk(
        EngineCreateInfo,
        &m_pDevice,
        &m_pDeviceContext);

    if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
        return E_FAIL;

    Diligent::SwapChainDesc SwapChainDesc{};
    SwapChainDesc.Width = m_tEngineDesc.iWinSizeX;
    SwapChainDesc.Height = m_tEngineDesc.iWinSizeY;

    const Diligent::Win32NativeWindow NativeWindow{m_tEngineDesc.pNativeWindow};
    pEngineFactory->CreateSwapChainVk(
        m_pDevice,
        m_pDeviceContext,
        SwapChainDesc,
        NativeWindow,
        &m_pSwapChain);

    return m_pSwapChain != nullptr ? S_OK : E_FAIL;
}

HRESULT CGraphicDevice::Initialize_DirectX11()
{
    Diligent::IEngineFactoryD3D11* pEngineFactory = Diligent::LoadAndGetEngineFactoryD3D11();
    if (pEngineFactory == nullptr)
        return E_FAIL;

    pEngineFactory->SetBreakOnError(false);

    Diligent::EngineD3D11CreateInfo EngineCreateInfo{};
    pEngineFactory->CreateDeviceAndContextsD3D11(
        EngineCreateInfo,
        &m_pDevice,
        &m_pDeviceContext);

    if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
        return E_FAIL;

    Diligent::SwapChainDesc SwapChainDesc{};
    SwapChainDesc.Width = m_tEngineDesc.iWinSizeX;
    SwapChainDesc.Height = m_tEngineDesc.iWinSizeY;

    const Diligent::Win32NativeWindow NativeWindow{m_tEngineDesc.pNativeWindow};
    pEngineFactory->CreateSwapChainD3D11(
        m_pDevice,
        m_pDeviceContext,
        SwapChainDesc,
        Diligent::FullScreenModeDesc{},
        NativeWindow,
        &m_pSwapChain);

    return m_pSwapChain != nullptr ? S_OK : E_FAIL;
}

void CGraphicDevice::Begin_Render()
{
    if (m_pDeviceContext == nullptr || m_pSwapChain == nullptr)
        return;

    constexpr float vClearColor[] = {1.f,0.f,0.f, 1.f};
    auto* pRenderTarget = m_pSwapChain->GetCurrentBackBufferRTV();
    auto* pDepthTarget = m_pSwapChain->GetDepthBufferDSV();

    m_pDeviceContext->SetRenderTargets(
        1,
        &pRenderTarget,
        pDepthTarget,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    m_pDeviceContext->ClearRenderTarget(
        pRenderTarget,
        vClearColor,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    if (pDepthTarget != nullptr)
    {
        m_pDeviceContext->ClearDepthStencil(
            pDepthTarget,
            Diligent::CLEAR_DEPTH_FLAG,
            1.f,
            0,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}

void CGraphicDevice::End_Render()
{
    if (m_pSwapChain != nullptr)
        m_pSwapChain->Present(1);
}

void CGraphicDevice::Resize(uint32 iWinSizeX, uint32 iWinSizeY)
{
    if (m_pSwapChain == nullptr || iWinSizeX == 0 || iWinSizeY == 0)
        return;

    m_tEngineDesc.iWinSizeX = iWinSizeX;
    m_tEngineDesc.iWinSizeY = iWinSizeY;
    m_pSwapChain->Resize(iWinSizeX, iWinSizeY);
}

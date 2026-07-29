#include "Engine_Android.h"

#include <memory>

#include "CGraphicDevice.h"

namespace
{
    std::unique_ptr<CGraphicDevice> g_pGraphicDevice;
#if defined(__ANDROID__)
    AAssetManager* g_pAssetManager = nullptr;
#endif
}

#if defined(__ANDROID__)
void Engine::Set_Android_AssetManager(AAssetManager* pAssetManager)
{
    g_pAssetManager = pAssetManager;
}

AAssetManager* Engine::Get_Android_AssetManager()
{
    return g_pAssetManager;
}
#endif
HRESULT Engine::Initialize_Android_Graphic(const Engine_Desc& Desc)
{
    if (Desc.eGraphicAPI != GRAPHIC_API::VULKAN || Desc.pNativeWindow == nullptr)
        return E_INVALIDARG;

    Release_Android_Graphic();

    auto pGraphicDevice = std::make_unique<CGraphicDevice>();
    const HRESULT hr = pGraphicDevice->Initialize(Desc);
    if (FAILED(hr))
        return hr;

    g_pGraphicDevice = std::move(pGraphicDevice);
    return S_OK;
}

void Engine::Draw_Android_Graphic()
{
    if (g_pGraphicDevice == nullptr)
        return;

    g_pGraphicDevice->Resize(0, 0);
    g_pGraphicDevice->Begin_Render();
    g_pGraphicDevice->End_Render();
}

void Engine::Release_Android_Graphic()
{
    g_pGraphicDevice.reset();
}


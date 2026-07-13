#pragma once

#include "Engine_Struct.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

class CGraphicDevice final
{
public:
    HRESULT Initialize(const Engine_Desc& tEngineDesc);
    void Begin_Render();
    void End_Render();
    void Resize(uint32 iWinSizeX, uint32 iWinSizeY);

private:
    HRESULT Initialize_Vulkan();
    HRESULT Initialize_DirectX11();

private:
    Engine_Desc m_tEngineDesc{};
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pDeviceContext;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;
};

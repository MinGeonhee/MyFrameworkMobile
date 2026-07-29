#pragma once

#include <string>
#include <vector>

#include "Engine_Platform.h"
#include "Engine_Define.h"
#include "Engine_Struct.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

class CGraphicDevice final
{
public:
    HRESULT Initialize(const Engine::Engine_Desc& tEngineDesc);
    void Begin_Render();
    void End_Render();
    void Resize(uint32 iWinSizeX, uint32 iWinSizeY);
    Diligent::IRenderDevice* Get_Device() const { return m_pDevice.RawPtr(); }
    Diligent::IDeviceContext* Get_DeviceContext() const { return m_pDeviceContext.RawPtr(); }
    Diligent::ISwapChain* Get_SwapChain() const { return m_pSwapChain.RawPtr(); }
    uint32 Get_Width() const { return m_tEngineDesc.iWinSizeX; }
    uint32 Get_Height() const { return m_tEngineDesc.iWinSizeY; }

private:
    HRESULT Initialize_Vulkan();
#if defined(_WIN32)
    HRESULT Initialize_DirectX11();
#endif

private:
    Engine::Engine_Desc m_tEngineDesc{};
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pDeviceContext;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;
};


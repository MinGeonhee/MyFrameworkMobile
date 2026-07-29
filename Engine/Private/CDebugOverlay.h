#pragma once

#include "CBase.h"
#include "Common/interface/BasicMath.hpp"
#include "Common/interface/RefCntAutoPtr.hpp"

class CShader;
class CTexture;

namespace Diligent
{
    struct IBuffer;
}

class CDebugOverlay final : public CBase
{
private:
    explicit CDebugOverlay(Device* pDevice, Context* pDeviceContext);
    ~CDebugOverlay() override = default;

public:
    HRESULT Initialize();
    HRESULT Render_FPS(uint32 iFPS, uint32 iScreenWidth, uint32 iScreenHeight);

    static CDebugOverlay* Create(Device* pDevice, Context* pDeviceContext);
    void Free() override;

private:
    HRESULT Draw_Character(char ch, float fX, float fY, float fScale, uint32 iScreenWidth, uint32 iScreenHeight);
    HRESULT Push_Rect(float fX, float fY, float fWidth, float fHeight, uint32 iScreenWidth, uint32 iScreenHeight,
                      float fU, float fV, float fUVWidth, float fUVHeight);
    HRESULT Flush();

private:
    Device* m_pDevice = nullptr;
    Context* m_pDeviceContext = nullptr;
    CShader* m_pShader = nullptr;
    CTexture* m_pNumberFontTexture = nullptr;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexBuffer;
    std::vector<Engine::VTXPOSTEX> m_Vertices;
};

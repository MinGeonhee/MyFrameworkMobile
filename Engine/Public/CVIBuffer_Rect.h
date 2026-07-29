#pragma once
#include "CVIBuffer.h"
#include "Common/interface/BasicMath.hpp"

class CShader;
class CTexture;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CVIBuffer_Rect final : public CVIBuffer
{
private:
    explicit CVIBuffer_Rect(Device* pDevice, Context* pDeviceContext);
    explicit CVIBuffer_Rect(const CVIBuffer_Rect& Prototype);
    ~CVIBuffer_Rect() override = default;

    HRESULT Proto_Initialize();
    HRESULT Clone_Initialize(void* pArg) override;

public:
    void Set_Shader(CShader* pShader);
    void Set_Texture(CTexture* pTexture, uint32 iTextureIndex = 0);
    void Set_UVRect(float fOffsetU, float fOffsetV, float fScaleU, float fScaleV);
    void Set_WorldViewProjection(const Diligent::float4x4& WorldViewProjection);
    void Set_ScreenSpace();
    HRESULT Render() override;
    HRESULT Render_Solid();
    static CVIBuffer_Rect* Create(Device* pDevice, Context* pDeviceContext);
    CComponent* Clone(void* pArg) override;
    void Free() override;

private:
    CShader* m_pShader = nullptr;
    CTexture* m_pTexture = nullptr;
    uint32 m_iTextureIndex = 0;
    float m_vUVRect[4]{0.f, 0.f, 1.f, 1.f};
    Diligent::float4x4 m_WorldViewProjection = Diligent::float4x4::Identity();
    float m_vTransformOption[4]{};
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "pch.h"
#include "CVIBuffer_Rect.h"
#include "CGameManager.h"
#include "CShader.h"
#include "CTexture.h"

CVIBuffer_Rect::CVIBuffer_Rect(Device* pDevice, Context* pDeviceContext)
    : CVIBuffer{pDevice, pDeviceContext}
{
}

CVIBuffer_Rect::CVIBuffer_Rect(const CVIBuffer_Rect& Prototype)
    : CVIBuffer{Prototype}
    , m_pShader{Prototype.m_pShader}
    , m_pTexture{Prototype.m_pTexture}
    , m_iTextureIndex{Prototype.m_iTextureIndex}
    , m_WorldViewProjection{Prototype.m_WorldViewProjection}
{
    std::copy(std::begin(Prototype.m_vUVRect), std::end(Prototype.m_vUVRect), std::begin(m_vUVRect));
    std::copy(std::begin(Prototype.m_vTransformOption), std::end(Prototype.m_vTransformOption), std::begin(m_vTransformOption));
    if (m_pShader != nullptr)
        m_pShader->AddRef();
    if (m_pTexture != nullptr)
        m_pTexture->AddRef();
}

HRESULT CVIBuffer_Rect::Proto_Initialize()
{
    constexpr Engine::VTXPOSTEX Vertices[] = {
        {{-0.5f, +0.5f, 0.f}, {0.f, 0.f}},
        {{+0.5f, +0.5f, 0.f}, {1.f, 0.f}},
        {{+0.5f, -0.5f, 0.f}, {1.f, 1.f}},
        {{-0.5f, -0.5f, 0.f}, {0.f, 1.f}}
    };

    constexpr uint16 Indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    if (FAILED(CVIBuffer::Proto_Initialize(
            Vertices,
            sizeof(Vertices),
            sizeof(Engine::VTXPOSTEX),
            static_cast<uint32>(std::size(Vertices)),
            Indices,
            sizeof(Indices),
            Diligent::VT_UINT16,
            static_cast<uint32>(std::size(Indices)),
            Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)))
        return E_FAIL;

    return S_OK;
}

HRESULT CVIBuffer_Rect::Clone_Initialize(void* pArg)
{
    return CVIBuffer::Clone_Initialize(pArg);
}

void CVIBuffer_Rect::Set_Shader(CShader* pShader)
{
    if (pShader == m_pShader) return;
    if (pShader != nullptr) pShader->AddRef();
    if (m_pShader != nullptr) m_pShader->Release();
    m_pShader = pShader;
}

void CVIBuffer_Rect::Set_Texture(CTexture* pTexture, uint32 iTextureIndex)
{
    if (pTexture != m_pTexture)
    {
        if (pTexture != nullptr) pTexture->AddRef();
        if (m_pTexture != nullptr) m_pTexture->Release();
        m_pTexture = pTexture;
    }
    m_iTextureIndex = iTextureIndex;
}

void CVIBuffer_Rect::Set_UVRect(float fOffsetU, float fOffsetV, float fScaleU, float fScaleV)
{
    m_vUVRect[0]=fOffsetU; m_vUVRect[1]=fOffsetV; m_vUVRect[2]=fScaleU; m_vUVRect[3]=fScaleV;
}

void CVIBuffer_Rect::Set_WorldViewProjection(const Diligent::float4x4& WorldViewProjection)
{
    // Diligent BasicMath는 CPU에서 Row Major로 계산하고 HLSL cbuffer에는 전치해서 전달한다.
    m_WorldViewProjection = WorldViewProjection.Transpose();
    m_vTransformOption[0] = 1.f;
}

void CVIBuffer_Rect::Set_ScreenSpace()
{
    m_WorldViewProjection = Diligent::float4x4::Identity();
    m_vTransformOption[0] = 0.f;
}

HRESULT CVIBuffer_Rect::Render()
{
    if (m_pDeviceContext == nullptr)
        return E_FAIL;

    if (m_pShader == nullptr)
    {
        // 원본처럼 VIBuffer가 Shader를 직접 만들지 않고 ResourceManager에서 공유해서 사용한다.
        m_pShader = static_cast<CShader*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(
            L"Shader_VIBuffer_Rect", RESOURCE_TYPE::SHADER, false));
    }
    if (m_pTexture == nullptr)
    {
        m_pTexture = static_cast<CTexture*>(CGameManager::GetInstance()->ResourceManager_Get_Resource(
            L"Texture_Default_White", RESOURCE_TYPE::TEXTURE, false));
    }
    struct SPRITE_TRANSFORM_CONSTANTS
    {
        Diligent::float4x4 WorldViewProjection;
        float TransformOption[4];
    } TransformConstants{m_WorldViewProjection, {
        m_vTransformOption[0], m_vTransformOption[1], m_vTransformOption[2], m_vTransformOption[3]}};

    if (m_pShader == nullptr || m_pTexture == nullptr ||
        FAILED(m_pShader->Set_GPU_Texture("g_Texture", m_pTexture->GetShaderResourceView(m_iTextureIndex))) ||
        FAILED(m_pShader->Set_GPU_ConstantBuffer("SpriteConstants", m_vUVRect, sizeof(m_vUVRect))) ||
        FAILED(m_pShader->Set_GPU_ConstantBuffer("SpriteTransformConstants", &TransformConstants, sizeof(TransformConstants))) ||
        FAILED(m_pShader->Apply_Pass("Default")))
        return E_FAIL;
    return CVIBuffer::Render();
}

HRESULT CVIBuffer_Rect::Render_Solid()
{
    if (m_pShader == nullptr)
        return E_FAIL;

    struct SPRITE_TRANSFORM_CONSTANTS
    {
        Diligent::float4x4 WorldViewProjection;
        float TransformOption[4];
    } TransformConstants{m_WorldViewProjection, {
        m_vTransformOption[0], m_vTransformOption[1], m_vTransformOption[2], m_vTransformOption[3]}};

    if (FAILED(m_pShader->Set_GPU_ConstantBuffer("SpriteTransformConstants", &TransformConstants, sizeof(TransformConstants))) ||
        FAILED(m_pShader->Apply_Pass("Default")))
        return E_FAIL;

    return CVIBuffer::Render();
}

CVIBuffer_Rect* CVIBuffer_Rect::Create(Device* pDevice, Context* pDeviceContext)
{
    CVIBuffer_Rect* pRect = new CVIBuffer_Rect{pDevice, pDeviceContext};
    if (FAILED(pRect->Proto_Initialize()))
    {
        pRect->Release();
        return nullptr;
    }
    return pRect;
}

CComponent* CVIBuffer_Rect::Clone(void* pArg)
{
    CVIBuffer_Rect* pRect = new CVIBuffer_Rect{*this};
    if (FAILED(pRect->Clone_Initialize(pArg)))
    {
        pRect->Release();
        return nullptr;
    }
    return pRect;
}

void CVIBuffer_Rect::Free()
{
    if (m_pShader != nullptr)
    {
        m_pShader->Release();
        m_pShader = nullptr;
    }
    if (m_pTexture != nullptr)
    {
        m_pTexture->Release();
        m_pTexture = nullptr;
    }
    CVIBuffer::Free();
}

#include "pch.h"
#include "CVIBuffer.h"

#include "Graphics/GraphicsEngine/interface/Buffer.h"

CVIBuffer::CVIBuffer(Device* pDevice, Context* pDeviceContext)
    : CComponent{pDevice, pDeviceContext}
{
}

CVIBuffer::CVIBuffer(const CVIBuffer& Prototype)
    : CComponent{Prototype}
    , m_iVertexStride{Prototype.m_iVertexStride}
    , m_iVertexCount{Prototype.m_iVertexCount}
    , m_iIndexCount{Prototype.m_iIndexCount}
    , m_eIndexType{Prototype.m_eIndexType}
    , m_ePrimitiveTopology{Prototype.m_ePrimitiveTopology}
    , m_pVertexBuffer{Prototype.m_pVertexBuffer}
    , m_pIndexBuffer{Prototype.m_pIndexBuffer}
{
}

HRESULT CVIBuffer::Proto_Initialize(const void* pVertexData,
                                    uint32 iVertexDataSize,
                                    uint32 iVertexStride,
                                    uint32 iVertexCount,
                                    const void* pIndexData,
                                    uint32 iIndexDataSize,
                                    Diligent::VALUE_TYPE eIndexType,
                                    uint32 iIndexCount,
                                    Diligent::PRIMITIVE_TOPOLOGY ePrimitiveTopology)
{
    if (m_pDevice == nullptr || pVertexData == nullptr || pIndexData == nullptr ||
        iVertexDataSize == 0 || iIndexDataSize == 0 || iVertexStride == 0 ||
        iVertexCount == 0 || iIndexCount == 0)
        return E_INVALIDARG;

    Diligent::BufferDesc VertexBufferDesc{};
    VertexBufferDesc.Name = "VIBuffer Vertex Buffer";
    VertexBufferDesc.Size = iVertexDataSize;
    VertexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
    VertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;

    Diligent::BufferData VertexBufferData{};
    VertexBufferData.pData = pVertexData;
    VertexBufferData.DataSize = iVertexDataSize;
    m_pDevice->CreateBuffer(VertexBufferDesc, &VertexBufferData, &m_pVertexBuffer);
    if (m_pVertexBuffer == nullptr)
        return E_FAIL;

    Diligent::BufferDesc IndexBufferDesc{};
    IndexBufferDesc.Name = "VIBuffer Index Buffer";
    IndexBufferDesc.Size = iIndexDataSize;
    IndexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
    IndexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;

    Diligent::BufferData IndexBufferData{};
    IndexBufferData.pData = pIndexData;
    IndexBufferData.DataSize = iIndexDataSize;
    m_pDevice->CreateBuffer(IndexBufferDesc, &IndexBufferData, &m_pIndexBuffer);
    if (m_pIndexBuffer == nullptr)
        return E_FAIL;

    m_iVertexStride = iVertexStride;
    m_iVertexCount = iVertexCount;
    m_iIndexCount = iIndexCount;
    m_eIndexType = eIndexType;
    m_ePrimitiveTopology = ePrimitiveTopology;
    return S_OK;
}

HRESULT CVIBuffer::Clone_Initialize(void* pArg)
{
    return CComponent::Clone_Initialize(pArg);
}

HRESULT CVIBuffer::Bind_Resource()
{
    if (m_pDeviceContext == nullptr || m_pVertexBuffer == nullptr || m_pIndexBuffer == nullptr)
        return E_FAIL;

    Diligent::IBuffer* pVertexBuffers[] = {m_pVertexBuffer.RawPtr()};
    constexpr Diligent::Uint64 Offsets[] = {0};
    m_pDeviceContext->SetVertexBuffers(
        0,
        1,
        pVertexBuffers,
        Offsets,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    m_pDeviceContext->SetIndexBuffer(
        m_pIndexBuffer,
        0,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    return S_OK;
}

HRESULT CVIBuffer::Draw()
{
    if (m_pDeviceContext == nullptr || m_iIndexCount == 0)
        return E_FAIL;

    Diligent::DrawIndexedAttribs DrawAttribs{};
    DrawAttribs.NumIndices = m_iIndexCount;
    DrawAttribs.IndexType = m_eIndexType;
    DrawAttribs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    m_pDeviceContext->DrawIndexed(DrawAttribs);
    return S_OK;
}

HRESULT CVIBuffer::Render()
{
    if (FAILED(Bind_Resource()))
        return E_FAIL;
    return Draw();
}

void CVIBuffer::Free()
{
    m_pIndexBuffer.Release();
    m_pVertexBuffer.Release();
    CComponent::Free();
}

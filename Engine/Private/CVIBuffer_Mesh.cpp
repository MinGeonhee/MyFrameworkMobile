#include "pch.h"
#include "CVIBuffer_Mesh.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(Device* pDevice, Context* pDeviceContext)
    : CVIBuffer{pDevice, pDeviceContext}
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype)
    : CVIBuffer{Prototype}
{
}

HRESULT CVIBuffer_Mesh::Proto_Initialize(const void* pVertexData,
                                         uint32 iVertexDataSize,
                                         uint32 iVertexStride,
                                         uint32 iVertexCount,
                                         const uint32* pIndices,
                                         uint32 iIndexCount)
{
    return CVIBuffer::Proto_Initialize(
        pVertexData, iVertexDataSize, iVertexStride, iVertexCount,
        pIndices, iIndexCount * sizeof(uint32), Diligent::VT_UINT32, iIndexCount,
        Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}

HRESULT CVIBuffer_Mesh::Clone_Initialize(void* pArg)
{
    return CVIBuffer::Clone_Initialize(pArg);
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(const void* pVertexData,
                                       uint32 iVertexDataSize,
                                       uint32 iVertexStride,
                                       uint32 iVertexCount,
                                       const uint32* pIndices,
                                       uint32 iIndexCount,
                                       Device* pDevice,
                                       Context* pDeviceContext)
{
    CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh{pDevice, pDeviceContext};
    if (FAILED(pInstance->Proto_Initialize(
            pVertexData, iVertexDataSize, iVertexStride, iVertexCount, pIndices, iIndexCount)))
    {
        pInstance->Release();
        return nullptr;
    }
    return pInstance;
}

CComponent* CVIBuffer_Mesh::Clone(void* pArg)
{
    CVIBuffer_Mesh* pCopy = new CVIBuffer_Mesh{*this};
    if (FAILED(pCopy->Clone_Initialize(pArg)))
    {
        pCopy->Release();
        return nullptr;
    }
    return pCopy;
}

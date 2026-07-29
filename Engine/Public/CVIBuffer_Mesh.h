#pragma once

#include "CVIBuffer.h"

class ENGINE_DLL CVIBuffer_Mesh final : public CVIBuffer
{
private:
    explicit CVIBuffer_Mesh(Device* pDevice, Context* pDeviceContext);
    explicit CVIBuffer_Mesh(const CVIBuffer_Mesh& Prototype);
    ~CVIBuffer_Mesh() override = default;

    HRESULT Proto_Initialize(const void* pVertexData,
                             uint32 iVertexDataSize,
                             uint32 iVertexStride,
                             uint32 iVertexCount,
                             const uint32* pIndices,
                             uint32 iIndexCount);
    HRESULT Clone_Initialize(void* pArg) override;

public:
    static CVIBuffer_Mesh* Create(const void* pVertexData,
                                  uint32 iVertexDataSize,
                                  uint32 iVertexStride,
                                  uint32 iVertexCount,
                                  const uint32* pIndices,
                                  uint32 iIndexCount,
                                  Device* pDevice,
                                  Context* pDeviceContext);
    CComponent* Clone(void* pArg) override;
};

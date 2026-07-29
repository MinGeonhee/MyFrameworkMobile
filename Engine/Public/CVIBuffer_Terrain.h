#pragma once
#include "CVIBuffer.h"

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
public:
    struct TERRAIN_BUFFER_DESC
    {
        uint32 iNumCellsX = 10;
        uint32 iNumCellsZ = 10;
        float fCellSize = 1.f;
    };
private:
    explicit CVIBuffer_Terrain(Device* pDevice, Context* pContext);
    explicit CVIBuffer_Terrain(const CVIBuffer_Terrain& Prototype);
    ~CVIBuffer_Terrain() override = default;
    HRESULT Proto_Initialize();
    HRESULT Clone_Initialize(void* pArg) override;
public:
    static CVIBuffer_Terrain* Create(Device* pDevice, Context* pContext);
    CComponent* Clone(void* pArg) override;
    void Free() override;
};

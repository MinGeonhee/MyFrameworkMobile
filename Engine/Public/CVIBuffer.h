#pragma once
#include "CComponent.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
    explicit CVIBuffer(Device* pDevice, Context* pDeviceContext);
    explicit CVIBuffer(const CVIBuffer& Prototype);
    ~CVIBuffer() override = default;

    HRESULT Proto_Initialize(const void* pVertexData,
                             uint32 iVertexDataSize,
                             uint32 iVertexStride,
                             uint32 iVertexCount,
                             const void* pIndexData,
                             uint32 iIndexDataSize,
                             Diligent::VALUE_TYPE eIndexType,
                             uint32 iIndexCount,
                             Diligent::PRIMITIVE_TOPOLOGY ePrimitiveTopology);
    HRESULT Clone_Initialize(void* pArg) override;

public:
    virtual HRESULT Bind_Resource();
    virtual HRESULT Draw();
    HRESULT Render() override;

protected:
    uint32 m_iVertexStride = 0;
    uint32 m_iVertexCount = 0;
    uint32 m_iIndexCount = 0;
    Diligent::VALUE_TYPE m_eIndexType = Diligent::VT_UNDEFINED;
    Diligent::PRIMITIVE_TOPOLOGY m_ePrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_UNDEFINED;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pVertexBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pIndexBuffer;

public:
    void Free() override;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "CVIBuffer_Cell.h"


CVIBuffer_Cell::CVIBuffer_Cell(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
    : CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Cell::CVIBuffer_Cell(const CVIBuffer_Cell& Proto)
    : CVIBuffer(Proto)
{
}

HRESULT CVIBuffer_Cell::Proto_Initialize(const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)])
{
    /* 무조건 시계방향으로 되어있다. */
    /* Cell 의 모든 정점을 들고와서 그려주면 되지 않을까 */
    /* 그럼이거 Navigation 그려줄 떄 해줘야하는 일... */

    m_iCountVertex = 3;
    m_iStrideVertex = sizeof(VTXPOSCOLOR);
    m_iOffsetVertex = 0;
    m_iCountVertexBuffer = 1;


    BUFFER_DESC tVertexDesc{};
    tVertexDesc.ByteWidth = m_iStrideVertex * m_iCountVertex;
    tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;
    tVertexDesc.CPUAccessFlags = 0;
    tVertexDesc.MiscFlags = 0;
    tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
    tVertexDesc.StructureByteStride = m_iStrideVertex;


    VTXPOSCOLOR* pVertices = new VTXPOSCOLOR[m_iCountVertex];
    m_pVertexPosition = new Float3[m_iCountVertex];
    
    for (uint32 i = 0; i < Cast_Uint(CELL_POINT::END); ++i)
    {
        m_pVertexPosition[i] = pVertices[i].vPosition = pVertexPos[i];
    }
    pVertices[0].vColor = { 1.0f, 0.7f, 0.7f, 1.0f };  // 밝은 코랄
    pVertices[1].vColor = { 0.7f, 1.0f, 0.7f, 1.0f };  // 라이트 라임
    pVertices[2].vColor = { 0.7f, 0.9f, 1.0f, 1.0f };  // 베이비 블루

    SUBRESOURCE_DATA tVertexData{};
    tVertexData.pSysMem = pVertices;

    HRESULT hr{ E_FAIL };

    hr = Create_Buffer(tVertexDesc, &tVertexData, &m_pVertexBuffer);


    Safe_Delete_Arr(pVertices);

    return S_OK;
}


HRESULT	CVIBuffer_Cell::Bind_Resource()
{
    IASetVertexBuffers(0, m_iCountVertexBuffer ,m_pVertexBuffer.GetAddressOf(),&m_iStrideVertex, &m_iOffsetVertex);
    IASetPrimitiveTopology(m_ePrimitiveType); 

    return S_OK;
}

HRESULT	CVIBuffer_Cell::Draw()
{
    Context_Draw(m_iCountVertex, 0);

    return S_OK;
}

void CVIBuffer_Cell::Change_Point()
{
    m_ePrimitiveType = TOPOLOGY_POINTLIST;
    m_iCountVertex = 1;
}

void CVIBuffer_Cell::Change_Line()
{
    m_ePrimitiveType = TOPOLOGY_LINELIST;
    m_iCountVertex = 2;
}

void CVIBuffer_Cell::Change_Tri()
{
    m_ePrimitiveType = TOPOLOGY_TRIANGLELIST;
    m_iCountVertex = 3;
}





HRESULT CVIBuffer_Cell::Clone_Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CVIBuffer_Cell::SubResourceUpdate(const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)])
{


    VTXPOSCOLOR* pVertices = new VTXPOSCOLOR[Cast_Uint(CELL_POINT::END)];
    
    for (uint32 i = 0; i < Cast_Uint(CELL_POINT::END); ++i)
        m_pVertexPosition[i] = pVertices[i].vPosition = pVertexPos[i];

    pVertices[0].vColor = { 1.0f, 0.7f, 0.7f, 1.0f };  // 밝은 코랄
    pVertices[1].vColor = { 0.7f, 1.0f, 0.7f, 1.0f };  // 라이트 라임
    pVertices[2].vColor = { 0.7f, 0.9f, 1.0f, 1.0f };  // 베이비 블루


    UpdateSubresource(m_pVertexBuffer.Get() , 0 , nullptr , pVertices, 0 , 0 );
    

    delete[] pVertices;


    return S_OK;
}

CVIBuffer_Cell* CVIBuffer_Cell::Create(const Float3 pVertexPos[Cast_Uint(CELL_POINT::END)], ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
    CVIBuffer_Cell* pCellBuffer = new CVIBuffer_Cell(pDevice, pContext);

    if (FAILED(pCellBuffer->Proto_Initialize(pVertexPos)))
    {
        Safe_Release(pCellBuffer);
        MSG_BOX(" VIBuffer_Cell is failed to Create ");
        return nullptr;
    }

    return pCellBuffer;
}


CComponent* CVIBuffer_Cell::Clone(void* pArg)
{
    CVIBuffer_Cell* pCellBufferClone = new CVIBuffer_Cell(*this);

    if (FAILED(pCellBufferClone->Clone_Initialize(pArg)))
    {
        Safe_Release(pCellBufferClone);
        MSG_BOX("VIBuffer_Cell is failed to Clone");
        return nullptr;
    }

    return pCellBufferClone;
}

void CVIBuffer_Cell::Free()
{
    __super::Free();

    /* CVIBuffer_Cell::Free() */
    


}



const INPUT_ELEMENT_DESC* CVIBuffer_Cell::GetInputElementDesc()
{
    return VTXPOSCOLOR().GetInputElementDesc().data();
}

uint32 CVIBuffer_Cell::GetInputElementDescSize()
{
    return VTXPOSCOLOR().GetInputElementDescSize();
}


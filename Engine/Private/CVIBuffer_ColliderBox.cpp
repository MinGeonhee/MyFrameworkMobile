#include "CVIBuffer_ColliderBox.h"


CVIBuffer_ColliderBox::CVIBuffer_ColliderBox(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_ColliderBox::~CVIBuffer_ColliderBox()
{
}

HRESULT CVIBuffer_ColliderBox::Proto_Initialize()
{
	/* 그냥 쉐이더 가 들고있는 콜라이더 근데이게 그 중점 구조체를 넘겨줘야하니까...*/
	/* 그치 Point라는 존 나 이상한 VIBuffer를 만들어버린거거든 */

	m_iStrideVertex = sizeof(VTXPOSCOLOR);
	m_iCountVertex = 8;
	m_iCountVertexBuffer = 1;
	m_iOffsetVertex = 0;							
	m_iStrideIndex = sizeof(uint16);				
	m_iCountIndex = 24;							
	m_iOffsetIndex = 0;							


	/* Line List  */
	m_ePrimitiveType = TOPOLOGY_LINELIST;


	VTXPOSCOLOR* pVertices = new VTXPOSCOLOR[m_iCountVertex];
	m_pVertexPosition = new Float3[m_iCountVertex];
	

	/* Front LT RT RB LB */
	m_pVertexPosition[0] = pVertices[LT].vPosition = { -1.f , +1.f , -1.f };
	m_pVertexPosition[1] = pVertices[RT].vPosition = { +1.f , +1.f , -1.f };
	m_pVertexPosition[2] = pVertices[RB].vPosition = { +1.f , -1.f , -1.f };
	m_pVertexPosition[3] = pVertices[LB].vPosition = { -1.f , -1.f , -1.f };


	m_pVertexPosition[4] = pVertices[4+LT].vPosition = { -1.f , +1.f , +1.f };
	m_pVertexPosition[5] = pVertices[4+RT].vPosition = { +1.f , +1.f , +1.f };
	m_pVertexPosition[6] = pVertices[4+RB].vPosition = { +1.f , -1.f , +1.f };
	m_pVertexPosition[7] = pVertices[4+LB].vPosition = { -1.f , -1.f , +1.f };

	BUFFER_DESC		tVertexDesc{};
	tVertexDesc.ByteWidth = m_iStrideVertex * m_iCountVertex; 
	tVertexDesc.StructureByteStride = m_iStrideVertex; 
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;	
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
	tVertexDesc.CPUAccessFlags = 0;
	tVertexDesc.MiscFlags = 0;	

	SUBRESOURCE_DATA tVertexData{};
	tVertexData.pSysMem = pVertices;

	
	/* Line Index 개수 12개  */
	UINT16* pIndices = new UINT16[m_iCountIndex];
	m_pIndices = new UINT16[m_iCountIndex];

	/* 윗면 */
	pIndices[0] = 4 + LT;	pIndices[1] = 4 + RT;
	pIndices[2] = 4 + RT;	pIndices[3] = RT;
	pIndices[4] =  RT   ;	pIndices[5] = LT;
	pIndices[6] =  LT   ;	pIndices[7] = 4+LT;

	/* 기둥 앞 */
	pIndices[8] =   LT  ;	pIndices[9] = LB;
	pIndices[10] =  RT  ;	pIndices[11] = RB;

	/* 기둥 뒤 */
	pIndices[12] = 4+LT  ;	pIndices[13] = 4 + LB;
	pIndices[14] = 4 +RT;	pIndices[15] = 4 + RB;

	/* 아랫면 */
	pIndices[16] = 4 + LB;	pIndices[17] = 4 + RB;
	pIndices[18] = 4 + RB;	pIndices[19] = RB;
	pIndices[20] = RB    ;	pIndices[21] = LB;
	pIndices[22] = LB    ;	pIndices[23] = 4 + LB;

	memcpy(reinterpret_cast<void*>(m_pIndices) , pIndices , sizeof(uint16) * m_iCountIndex);

	BUFFER_DESC		tIndexDesc{};
	tIndexDesc.ByteWidth = m_iStrideIndex * m_iCountIndex;
	tIndexDesc.StructureByteStride = m_iStrideIndex;
	tIndexDesc.Usage = BUFFER_USAGE_DEFAULT;
	tIndexDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;
	tIndexDesc.CPUAccessFlags = 0;
	tIndexDesc.MiscFlags = 0;

	SUBRESOURCE_DATA tIndexData{};
	tIndexData.pSysMem = pIndices;

	if (FAILED(Create_Buffer(tVertexDesc, &tVertexData, &m_pVertexBuffer)))
		return E_FAIL;
	if (FAILED(Create_Buffer(tIndexDesc, &tIndexData, &m_pIndexBuffer)))
		return E_FAIL;


	delete[] pVertices;
	delete[] pIndices;

    return S_OK;
}

HRESULT CVIBuffer_ColliderBox::Clone_Initialize(void* pArg)
{

    return S_OK;
}

CVIBuffer_ColliderBox* CVIBuffer_ColliderBox::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_ColliderBox* pVIBuffer_Collider = new CVIBuffer_ColliderBox(pDevice, pContext);

	if (FAILED(pVIBuffer_Collider->Proto_Initialize()))
	{
		Safe_Release(pVIBuffer_Collider);
		MSG_BOX("VIBuffer is failed to Create ");
		return nullptr;
	}

	return pVIBuffer_Collider;
}

CComponent* CVIBuffer_ColliderBox::Clone(void* pArg)
{
	CVIBuffer_ColliderBox* pVIBuffer_Collider = new CVIBuffer_ColliderBox(*this);

	if (FAILED(pVIBuffer_Collider->Clone_Initialize(pArg)))
	{
		Safe_Release(pVIBuffer_Collider);
		MSG_BOX("VIBuffer is failed to Clone ");
		return nullptr;
	}

	return pVIBuffer_Collider;
}

void CVIBuffer_ColliderBox::Free()
{
	__super::Free();



	/* CVIBuffer_Point::Free() */
}

const INPUT_ELEMENT_DESC* CVIBuffer_ColliderBox::GetInputElementDesc()
{
    return VTXPOSCOLOR::tInputElement_Desc.data();
}

uint32 CVIBuffer_ColliderBox::GetInputElementDescSize()
{
	return Cast_Uint(VTXPOSCOLOR::tInputElement_Desc.size());

}

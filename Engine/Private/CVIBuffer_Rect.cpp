#include "CVIBuffer_Rect.h"

CVIBuffer_Rect::CVIBuffer_Rect(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Rect::CVIBuffer_Rect(const CVIBuffer_Rect& ProtoType)
	: CVIBuffer(ProtoType)
{
}

HRESULT CVIBuffer_Rect::Proto_Initialize()
{
	m_iStrideVertex = sizeof(VTXPOSTEX);			// 내가 사용할 정점데이터 하나의 크기
	m_iCountVertex = 4;							// 정점 데이터의 개수 
	m_iOffsetVertex = 0;							// 시작 정점 데이터의 위치
	m_iStrideIndex = sizeof(uint16);				// Stride=> Index 정점 하나의 크기 2byte , 4byte 둘 중 하나를 선택 , 정점개수가 대략 65000개를 넘어가면 4바이트 사용
	m_iCountIndex = 6;							// index 데이터의 개수
	m_iOffsetIndex = 0;							// 시작 Index 데이터의 위치


	


#pragma region Vertex Index Create
	/* Vertex Buffer Data */
	VTXPOSTEX* pVertices = new VTXPOSTEX[m_iCountVertex];
	m_pVertexPosition = new Float3[m_iCountVertex];

	/* LT -> RT -> RB -> LB 수순 */
	/* Vertices에서 Position 에 대한 정보만 따로 저장해준다 UV는 사용할 일이 없다. */
	/* 지형탈떄 사용할듯? 정점 접근해서? */
	m_pVertexPosition[0] = pVertices[LT].vPosition = Float3(-0.5f, +0.5f, 0.f);
	pVertices[LT].vTexcoord = { 0.f , 0.f };

	m_pVertexPosition[1] = pVertices[RT].vPosition = Float3(+0.5f, +0.5f, 0.f);
	pVertices[RT].vTexcoord = { 1.f , 0.f };

	m_pVertexPosition[2] = pVertices[RB].vPosition = Float3(+0.5f, -0.5f, 0.f);
	pVertices[RB].vTexcoord = { 1.f , 1.f };

	m_pVertexPosition[3] = pVertices[LB].vPosition = Float3(-0.5f, -0.5f, 0.f);
	pVertices[LB].vTexcoord = { 0.f , 1.f };


	m_iCountVertexBuffer = 1;

	/* Index Buffer Data */
	m_pIndices = new uint16[m_iCountIndex];
	uint16* pIndices = static_cast<uint16*>(m_pIndices);

	pIndices[0] = 0; pIndices[1] = 1; pIndices[2] = 2;
	pIndices[3] = 0; pIndices[4] = 2; pIndices[5] = 3;

	

#pragma endregion

	//typedef struct BUFFER_DESC
	//{
	//	UINT ByteWidth;
	//	D3D11_USAGE Usage;
	//	UINT BindFlags;
	//	UINT CPUAccessFlags;
	//	UINT MiscFlags;
	//	UINT StructureByteStride;
	//} 	BUFFER_DESC;

#pragma region Vertex Desc
	BUFFER_DESC		tVertexDesc{};

	tVertexDesc.ByteWidth = m_iStrideVertex * m_iCountVertex; // 총 메모리 크기 한 정점 데이터의 크기 * 정점 데이터 개수
	tVertexDesc.StructureByteStride = m_iStrideVertex; // 정점 데이터 하나의 크기를 넣어준다.
	tVertexDesc.Usage = BUFFER_USAGE_DEFAULT;	  // 이 정점 데이터를 접근 권하을 줄것인가.
	//enum D3D11_USAGE
	//{
	//	BUFFER_USAGE_DEFAULT = 0,	=> GPU에서 읽기만 가능한 Default 상태 읽는 상태가 빠르다
	//	BUFFER_USAGE_IMMUTABLE = 1,	=> GPU에서
	//	BUFFER_USAGE_DYNAMIC = 2,	==> CPU 에서 접근을 가능하게 해준다 부분 접근보다 정점데이를 전부 틍으로 바꿀 이유가 있는 객체가 사용할 수 있다.
	//	BUFFER_USAGE_STAGING = 3		=> Dynamic과 마찬가지로 접근을 가능하게 해준다 대신 부분 접근에 있어서 빠른 속도를 보인다
	//} 	D3D11_USAGE;
	tVertexDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER; //
	/* BUFFER_BIND_FLAG => 이 정점 데이트를 사용할 사용처를 뭍고 있는것 Vertex면 Vertex , Index면 Index Bind를 해주면된다. */
	tVertexDesc.CPUAccessFlags = 0;  // => CPU접근권하 관련 설정 Dynamic같이 정점데이터가 변경될때 설정해줄 Flag
	tVertexDesc.MiscFlags = 0;		// -> CPUAccesFlag 와 비슷하다.

#pragma endregion
#pragma region Vertex Data

	//typedef struct d3d11_subresource_data
	//{
	//	const void* psysmem;				// 내가 사용하 Vertex 배열의 첫 시작부분을 넘겨준다.
	//	uint sysmempitch;					// 나중에 Texture 관련해서 넣어줄 데이터들
	//	uint sysmemslicepitch;				// 나중에 Texture 관련해서 넣어줄 데이터들
	//} 	d3d11_subresource_data;

	SUBRESOURCE_DATA	tVertexData{};
	tVertexData.pSysMem = pVertices;		// Vertex 배열의 시작 주소를 넣어준다.


#pragma endregion
#pragma region Index Desc
	/* Index Desc 는 Vertex Desc 와 플래그 값은 거의 일치한다 */
	BUFFER_DESC		tIndexDesc{};

	tIndexDesc.ByteWidth = m_iStrideIndex * m_iCountIndex;
	tIndexDesc.StructureByteStride = m_iStrideIndex;
	tIndexDesc.Usage = BUFFER_USAGE_DEFAULT;				// 일단 GPU말고는 접근을 막는다.
	tIndexDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;; //
	tIndexDesc.CPUAccessFlags = 0;
	tIndexDesc.MiscFlags = 0;
#pragma endregion
#pragma region Index Data

	SUBRESOURCE_DATA	tIndexData{};
	tIndexData.pSysMem = pIndices;			// Index 배열의 첫 시작 주소를 넣어준다.

#pragma endregion

	if (FAILED(CVIBuffer::Proto_Initialize(tVertexDesc, tVertexData, tIndexDesc, tIndexData, TOPOLOGY_TRIANGLELIST)))
		return E_FAIL;

	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;

	return S_OK;

}


HRESULT CVIBuffer_Rect::Clone_Initialize(void* pArg)
{
	if (FAILED(__super::Clone_Initialize(pArg)))
		return E_FAIL;
	

	/* CVIBuffer_Rect Clone_Initialize(pArg)*/



	return S_OK;
}




CVIBuffer_Rect* CVIBuffer_Rect::Create(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_Rect* pVIBuffer_Rect = new CVIBuffer_Rect(pDevice, pContext);

	if (FAILED(pVIBuffer_Rect->Proto_Initialize()))
	{
		Safe_Release(pVIBuffer_Rect);
		MSG_BOX("VIBuffer is failed to Create ");
		return nullptr;
	}

	return pVIBuffer_Rect;
}

CComponent* CVIBuffer_Rect::Clone(void* pArg)
{
	CVIBuffer_Rect* pVIBuffer_Rect = new CVIBuffer_Rect(*this);

	if (FAILED(pVIBuffer_Rect->Clone_Initialize(pArg)))
	{
		Safe_Release(pVIBuffer_Rect);
		MSG_BOX("VIBuffer is failed to Clone ");
		return nullptr;
	}

	return pVIBuffer_Rect;
}

void CVIBuffer_Rect::Free()
{
	__super::Free();

	/* CVIBuffer_Rect::Free() */
}

const INPUT_ELEMENT_DESC* CVIBuffer_Rect::GetInputElementDesc()
{
	return VTXPOSTEX().tInputElement_Desc.data();
}

uint32 CVIBuffer_Rect::GetInputElementDescSize()
{
	return Cast_Uint(VTXPOSTEX().tInputElement_Desc.size());
}

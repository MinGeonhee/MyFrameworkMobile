#include <filesystem>
#include "CVIBuffer_Terrain.h"
#include "CNavigation.h"


CVIBuffer_Terrain::CVIBuffer_Terrain(ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
	: CVIBuffer(pDevice, pContext), m_iCountVerticesX{}, m_iCountVerticesZ{}
{
}

CVIBuffer_Terrain::CVIBuffer_Terrain(const CVIBuffer_Terrain& ProtoType)
    : CVIBuffer(ProtoType) , m_iCountVerticesX{ ProtoType.m_iCountVerticesX }
	, m_iCountVerticesZ(ProtoType.m_iCountVerticesZ)
{
}


HRESULT CVIBuffer_Terrain::Proto_Initialize(CONST std::wstring& wstrPath)
{
	/* 파일을 먼저 열어준다 */

	/* 원본 : CreateFile / ReadFile (Win32) → 엔진 파일 스트림 (Android asset 지원) */
	CFileStream     hFile(wstrPath);
	if (!hFile.is_open())
		return E_FAIL;

	BITMAPFILEHEADER        tFileHeader{};
	BITMAPINFOHEADER        tInfoHeader{};
	uint32* pPixels = { nullptr };

	hFile.read(&tFileHeader, sizeof(tFileHeader));
	hFile.read(&tInfoHeader, sizeof(tInfoHeader));

	pPixels = new uint32[tInfoHeader.biWidth * tInfoHeader.biHeight];
	hFile.read(pPixels, sizeof(uint32) * tInfoHeader.biWidth * tInfoHeader.biHeight);


	/* 정점의 담아줄 버퍼의 개수 */
	m_iCountVertexBuffer = 1;
	/* 높이맵 파일의 가로 정점 개수 및 세로 정점 개수 */
	m_iCountVerticesX = tInfoHeader.biWidth;
	m_iCountVerticesZ = abs(tInfoHeader.biHeight);
	m_iCountVertex    = m_iCountVerticesX * m_iCountVerticesZ;
	


	/* 정점하나에 들어가는 데이터 크기 */
	m_iStrideVertex = sizeof(VTXNORTEX);
	m_iOffsetVertex = 0;
	/* Offset 0 */

	/* 인덱스 개수  = 정점 - 1 * 정점 - 1  * 3 * 3 */
	m_iCountIndex = (m_iCountVerticesX - 1) * (m_iCountVerticesZ -1) * 2 * 3;
	m_iStrideIndex = 4;
	/* 그리는 방식 트라이앵글 리스트 */
	m_ePrimitiveType = TOPOLOGY_TRIANGLELIST;


#pragma region VertexData
	BUFFER_DESC           VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iStrideVertex * m_iCountVertex;
	VertexBufferDesc.Usage = BUFFER_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = BUFFER_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iStrideVertex;

	VTXNORTEX* pVertices = new VTXNORTEX[m_iCountVertex];
	m_pNormal = new Float3[m_iCountVertex];
	m_pVertexPosition = new Float3[m_iCountVertex];
	NULLPTR_RETURN_EFAIL(pVertices);

	int64 iIndex{0};

	for (int64 z = 0 ; z < m_iCountVerticesZ; ++z)
	{
		for (int64 x = 0; x < m_iCountVerticesX; ++x)
		{
			iIndex =  x + m_iCountVerticesX * z;

			/* 비트맵 비트연산자 고고링 */
			m_pVertexPosition[iIndex] = pVertices[iIndex].vPosition = 
				{ (float)x , (pPixels[iIndex] & 0x000000ff ) / 10.f , (float)z};
			m_pNormal[iIndex] = pVertices[iIndex].vNormal = Float3(0.f, 0.f, 0.f);
			/* index 번호로 쳤을 떄  x == m_iCountVerticesX 랑 같아야지 1 이나오니 UV 좌표가 0 ~ 1 사이 값으로 매핑된다 */
			/* UV좌표에서 V좌표도 마찬가지로  근데이건단점이 그 텍스처가 거꾸로 나오게 되어버림 */
			pVertices[iIndex].vTexcoord = Float2((float)x / (m_iCountVerticesX - 1.f ) , (float)z / (m_iCountVerticesZ -1.f ));
		}
	}
	SUBRESOURCE_DATA      VertexInitialData{};
	VertexInitialData.pSysMem =	pVertices;

#pragma endregion

	BUFFER_DESC           IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iStrideIndex * m_iCountIndex;
	IndexBufferDesc.Usage = BUFFER_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = BUFFER_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iStrideIndex;

	uint32* pIndices = new uint32[m_iCountIndex];
	NULLPTR_RETURN_EFAIL(pIndices);

	iIndex = 0;
	int64 iCount{ 0 };
	for (int64 Z = 0; Z < m_iCountVerticesZ - 1; ++Z)
	{
		for( int64 X = 0 ; X < m_iCountVerticesX -1 ; ++X)
		{
			iIndex = Z * m_iCountVerticesX + X ;

			int64       iIndices[4] = {
				iIndex + m_iCountVerticesX,
				iIndex + m_iCountVerticesX + 1,
				iIndex + 1,
				iIndex
			};

			pIndices[iCount++] = iIndices[0];
			pIndices[iCount++] = iIndices[1];
			pIndices[iCount++] = iIndices[2];


			Vector4     vSour, vDest, vNormal;
			vSour = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);

			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[1]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);

			pIndices[iCount++] = iIndices[0];
			pIndices[iCount++] = iIndices[2];
			pIndices[iCount++] = iIndices[3];

			vSour = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);

			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[3]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);


			/* 법선벡터를 채워주자 */
			/* 점 각각 두개와 방향구하기 */

			//Vector4 vPos0 = XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			//Vector4 vPos1 = XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
			//Vector4 vPos2 = XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
			//Vector4 vPos3 = XMLoadFloat3(&pVertices[iIndices[3]].vPosition);

			//Vector4 vDir_1_0 = XMVectorSubtract( vPos1 , vPos0);
			//Vector4 vDir_2_0 = XMVectorSubtract(vPos2 , vPos0);

			//Vector4 vDir_3_0 = XMVectorSubtract(vPos0, vPos3);
			//Vector4 vDir_3_2 = XMVectorSubtract(vPos2, vPos3);


			//Vector4  tCross = XMVector3Normalize(XMVector3Cross(vDir_1_0, vDir_2_0));
			//XMStoreFloat3(&pVertices[iIndices[0]].vNormal,XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + tCross);
			//XMStoreFloat3(&pVertices[iIndices[1]].vNormal,XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + tCross);
			//XMStoreFloat3(&pVertices[iIndices[2]].vNormal,XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + tCross);

			//tCross = XMVector3Normalize(XMVector3Cross(vDir_3_0, vDir_3_2));
			//XMStoreFloat3(&pVertices[iIndices[0]].vNormal, XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + tCross);
			//XMStoreFloat3(&pVertices[iIndices[2]].vNormal, XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + tCross);
			//XMStoreFloat3(&pVertices[iIndices[3]].vNormal, XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + tCross);

		}
	}

	for (uint32 i = 0; i < m_iCountVertex; ++i)
	{
		XMStoreFloat3(&pVertices[i].vNormal,XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));
	}



	SUBRESOURCE_DATA      IndexInitialData{};
	IndexInitialData.pSysMem = pIndices;

	if (FAILED(CVIBuffer::Proto_Initialize(VertexBufferDesc, VertexInitialData, IndexBufferDesc, IndexInitialData, m_ePrimitiveType)))
		return E_FAIL;


	/* 힙메모리 할당 사용한 지역변수 제거 */
	delete[] pVertices;
	delete[] pIndices;
	delete[] pPixels;

    return S_OK;
}


HRESULT CVIBuffer_Terrain::Clone_Initialize(void* pArg)
{
	return S_OK;
}

CVIBuffer_Terrain* CVIBuffer_Terrain::Create( const std::wstring& wstrPath  ,ComPtr<Device> pDevice, ComPtr<DeviceContext> pContext)
{
	CVIBuffer_Terrain* pVIBuffer_Terrain = new CVIBuffer_Terrain(pDevice, pContext);

	if (FAILED(pVIBuffer_Terrain->Proto_Initialize(wstrPath)))
	{
		Safe_Release(pVIBuffer_Terrain);
		MSG_BOX("VIBuffer_Terrain is failed to Create ");
		return nullptr;
	}

	return pVIBuffer_Terrain;
}

CComponent* CVIBuffer_Terrain::Clone(void* pArg)
{
	CVIBuffer_Terrain* pVIBuffer_Terrain = new CVIBuffer_Terrain(*this);

	if (FAILED(pVIBuffer_Terrain->Clone_Initialize(pArg)))
	{
		Safe_Release(pVIBuffer_Terrain);
		MSG_BOX("VIBuffer_Terrain is failed to Clone ");
		return nullptr;
	}

	return pVIBuffer_Terrain;
}

void CVIBuffer_Terrain::Free()
{
	__super::Free();

	/* CVIBUffer_Terrain::Free() */

	if (m_eMakeType == MAKE_TYPE::PROTOTYPE)
		delete[] m_pVertexPosition;


}

const INPUT_ELEMENT_DESC* CVIBuffer_Terrain::GetInputElementDesc()
{
    return VTXNORTEX().GetInputElementDesc().data();
}

uint32 CVIBuffer_Terrain::GetInputElementDescSize()
{
    return VTXNORTEX().GetInputElementDescSize();
}
